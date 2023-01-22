from lexer import Token, TokenType


class ParserError(Exception):
    pass


class Member:
    def __init__(self, name: str, by_move: bool, type_: str) -> None:
        self.name = name
        self.by_move = by_move
        self.type_ = type_

    def __str__(self):
        return f"{self.name}{' by_move' if self.by_move else ''}: {self.type_}"


class PolymorphicType:
    def __init__(self, members: list[Member]) -> None:
        self.members = members

    def __str__(self):
        return "\n".join(f"\t\t\t{str(member)}" for member in self.members)


class GeneratorDescription:
    def __init__(self, includes: str, type_definitions: list[tuple[str, str]],
                 polymorphic_types: dict[str, dict[str, PolymorphicType]]) -> None:
        self.includes = includes
        self.type_definitions = type_definitions
        self.polymorphic_types = polymorphic_types

    def __str__(self):
        result = f"Includes:\n{self.includes}\n";
        result += "Type Definitions:\n" + "\n".join(
            str(definition) for definition in self.type_definitions) + "\n\nPolymorphic types:\n"
        for name, type_ in self.polymorphic_types.items():
            result += f"\t{name}:\n"
            for member_name, member_type in type_.items():
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

    def is_end_of_input(self):
        return self.index >= len(self.tokens) or self.current().type_ == TokenType.EndOfInput

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
    polymorphic_types: dict[str, dict[str, PolymorphicType]] = dict()
    if parser.current().type_ == TokenType.StringLiteral:
        includes = parser.current().lexeme[1:-1].strip()
        parser.next()  # consume includes
    else:
        includes = ""
    while not parser.is_end_of_input():
        match parser.current().type_:
            case TokenType.Type:
                parser.next()  # consume "type"
                identifier = parser.consume(TokenType.Identifier)
                contents = parser.consume(TokenType.StringLiteral).lexeme[1:-1].strip()
                type_definitions.append((identifier.lexeme, contents))
            case TokenType.Identifier:
                identifier = parser.consume(TokenType.Identifier)
                parser.consume(TokenType.Equals)
                sub_types: dict[str, PolymorphicType] = dict()
                name, polymorphic_type = parse_polymorphic_type(parser)
                sub_types[name] = polymorphic_type
                while parser.current().type_ == TokenType.Pipe:
                    parser.next()  # consume "|"
                    name, polymorphic_type = parse_polymorphic_type(parser)
                    sub_types[name] = polymorphic_type
                polymorphic_types[identifier.lexeme] = sub_types
            case _:
                raise ParserError(f'unexpected token "{parser.current().type_}"')
    return GeneratorDescription(includes, type_definitions, polymorphic_types)


def parse_polymorphic_type(parser: Parser) -> tuple[str, PolymorphicType]:
    identifier = parser.consume(TokenType.Identifier)
    parser.consume(TokenType.LeftParenthesis)
    members: list[Member] = list()
    while parser.current().type_ == TokenType.Identifier:
        member_name = parser.consume(TokenType.Identifier)
        by_move = parser.current().type_ == TokenType.ByMove
        if by_move:
            parser.next()  # consume "by_move"
        member_type = parser.consume(TokenType.StringLiteral).lexeme[1:-1].strip()
        members.append(Member(member_name.lexeme, by_move, member_type))
    parser.consume(TokenType.RightParenthesis)
    return identifier.lexeme, PolymorphicType(members)
