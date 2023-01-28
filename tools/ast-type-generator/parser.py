from lexer import Token, TokenType


class ParserError(Exception):
    pass


class Member:
    def __init__(self, name: str, by_move: bool, type_: str) -> None:
        self.name = name
        self.by_move = by_move
        self.type_ = type_

    def __str__(self) -> str:
        return f"{self.name}{' by_move' if self.by_move else ''}: {self.type_}"


class PureVirtualFunction:
    def __init__(self, name: str, return_type: str | None) -> None:
        self.name = name
        self.return_type = return_type


class Implementation:
    def __init__(self, name: str, body: str) -> None:
        self.name = name
        self.body = body


class PolymorphicType:
    def __init__(self, pure_virtual_functions: list[PureVirtualFunction], members: list[Member],
                 implementations: list[Implementation]) -> None:
        if len(set(function.name for function in pure_virtual_functions)) != len(
                [function.name for function in pure_virtual_functions]):
            raise ParserError("duplicate declaration of pure virtual functions")
        if len(set(function.name for function in implementations)) != len(
                [function.name for function in implementations]):
            raise ParserError("duplicate implementation")
        if set(function.name for function in pure_virtual_functions) != set(
                function.name for function in implementations):
            raise ParserError("not all pure virtual functions are implemented")

        self.pure_virtual_functions = pure_virtual_functions
        self.members = members
        self.implementations = implementations

    def __str__(self) -> str:
        return "\n".join(f"\t\t\t{str(member)}" for member in self.members)


class AbstractType:
    def __init__(self, sub_types: dict[str, PolymorphicType], pure_virtual_functions: list[PureVirtualFunction]) -> None:
        self.sub_types = sub_types
        self.pure_virtual_functions = pure_virtual_functions


class GeneratorDescription:
    def __init__(self, includes: str, type_definitions: list[tuple[str, str]],
                 abstract_types: dict[str, AbstractType]) -> None:
        self.includes = includes
        self.type_definitions = type_definitions
        self.abstract_types = abstract_types

    def __str__(self) -> str:
        result = f"Includes:\n{self.includes}\n";
        result += "Type Definitions:\n" + "\n".join(
            str(definition) for definition in self.type_definitions) + "\n\nPolymorphic types:\n"
        for name, type_ in self.abstract_types.items():
            result += f"\t{name}:\n"
            for member_name, member_type in type_.sub_types.items():
                result += f"\t\t{member_name}:\n{member_type}\n"
        return result


class Parser:
    def __init__(self, tokens: list[Token]) -> None:
        self.tokens = tokens
        self.index = 0

    def current(self) -> Token:
        return self.tokens[self.index]

    def next(self) -> None:
        self.index += 1

    def peek(self) -> Token | None:
        if self.is_end_of_input():
            return None
        return self.tokens[self.index + 1]

    def is_end_of_input(self) -> bool:
        return self.index >= len(self.tokens) or self.current().type_ == TokenType.END_OF_INPUT

    def consume(self, type_: TokenType) -> Token:
        if self.is_end_of_input():
            raise ParserError("unexpected end of input")
        if self.current().type_ != type_:
            raise ParserError(
                f'unexpected token type (expected {type_}, got {self.current().type_} ["{self.current().lexeme}"])')
        result = self.current()
        self.next()
        return result


def parse(tokens: list[Token]) -> GeneratorDescription:
    parser = Parser(tokens)
    type_definitions: list[tuple[str, str]] = list()
    polymorphic_types: dict[str, AbstractType] = dict()
    if parser.current().type_ == TokenType.STRING_LITERAL:
        includes = parser.current().lexeme[1:-1].strip()
        parser.next()  # consume includes
    else:
        includes = ""
    while not parser.is_end_of_input():
        match parser.current().type_:
            case TokenType.TYPE:
                parser.next()  # consume "type"
                identifier = parser.consume(TokenType.IDENTIFIER)
                contents = parser.consume(TokenType.STRING_LITERAL).lexeme[1:-1].strip()
                type_definitions.append((identifier.lexeme, contents))
            case TokenType.IDENTIFIER:
                identifier = parser.consume(TokenType.IDENTIFIER)
                parser.consume(TokenType.LEFT_PARENTHESIS)

                functions: list[PureVirtualFunction] = list()
                while parser.current().type_ == TokenType.FUNCTION:
                    parser.consume(TokenType.FUNCTION)
                    function_identifier = parser.consume(TokenType.IDENTIFIER).lexeme
                    return_type_string = parser.consume(TokenType.STRING_LITERAL).lexeme[1:-1].strip()
                    function_return_type = return_type_string if len(return_type_string) > 0 else None
                    functions.append(PureVirtualFunction(function_identifier, function_return_type))

                parser.consume(TokenType.RIGHT_PARENTHESIS)
                parser.consume(TokenType.EQUALS)
                sub_types: dict[str, PolymorphicType] = dict()
                name, members, implementations = parse_subtype(parser)
                polymorphic_type = PolymorphicType(functions, members, implementations)
                sub_types[name] = polymorphic_type
                while parser.current().type_ == TokenType.PIPE:
                    parser.next()  # consume "|"
                    name, members, implementations = parse_subtype(parser)
                    polymorphic_type = PolymorphicType(functions, members, implementations)
                    sub_types[name] = polymorphic_type
                polymorphic_types[identifier.lexeme] = AbstractType(sub_types, functions)
            case _:
                raise ParserError(f'unexpected token "{parser.current().type_}"')
    return GeneratorDescription(includes, type_definitions, polymorphic_types)


def parse_subtype(parser: Parser) -> tuple[str, list[Member], list[Implementation]]:
    identifier = parser.consume(TokenType.IDENTIFIER)
    parser.consume(TokenType.LEFT_PARENTHESIS)
    members: list[Member] = list()
    implementations: list[Implementation] = list()
    while True:
        if parser.current().type_ == TokenType.IDENTIFIER:
            member_name = parser.consume(TokenType.IDENTIFIER)
            by_move = parser.current().type_ == TokenType.BY_MOVE
            if by_move:
                parser.next()  # consume "by_move"
            member_type = parser.consume(TokenType.STRING_LITERAL).lexeme[1:-1].strip()
            members.append(Member(member_name.lexeme, by_move, member_type))
        elif parser.current().type_ == TokenType.IMPLEMENT:
            parser.consume(TokenType.IMPLEMENT)
            function_name = parser.consume(TokenType.IDENTIFIER).lexeme
            function_body = parser.consume(TokenType.STRING_LITERAL).lexeme[1:-1].strip()
            implementations.append(Implementation(function_name, function_body))
        else:
            break
    parser.consume(TokenType.RIGHT_PARENTHESIS)
    return identifier.lexeme, members, implementations
