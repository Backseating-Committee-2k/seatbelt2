from enum import Enum


class LexerError(Exception):
    pass


class TokenType(Enum):
    TYPE = 0
    IDENTIFIER = 1
    STRING_LITERAL = 2
    EQUALS = 3
    LEFT_PARENTHESIS = 4
    RIGHT_PARENTHESIS = 5
    PIPE = 6
    BY_MOVE = 7
    FUNCTION = 8
    IMPLEMENT = 9
    END_OF_INPUT = 10


class Token:
    def __init__(self, lexeme: str, type_: TokenType) -> None:
        self.lexeme = lexeme
        self.type_ = type_

    def __str__(self) -> str:
        return f"{self.type_}(\"{self.lexeme}\")"


def tokenize(input_: str) -> list[Token]:
    lexer = Lexer(input_)
    tokens = list()
    while not lexer.is_end_of_input():
        match lexer.current():
            case "(":
                tokens.append(Token(lexer.current(), TokenType.LEFT_PARENTHESIS))
                lexer.next()
            case ")":
                tokens.append(Token(lexer.current(), TokenType.RIGHT_PARENTHESIS))
                lexer.next()
            case "=":
                tokens.append(Token(lexer.current(), TokenType.EQUALS))
                lexer.next()
            case "|":
                tokens.append(Token(lexer.current(), TokenType.PIPE))
                lexer.next()
            case "{":
                tokens.append(string_literal(lexer))
            case "/":
                comment(lexer)
            case _:
                if lexer.current().isspace():
                    lexer.next()
                elif is_valid_identifier_start(lexer.current()):
                    tokens.append(identifier_or_keyword(lexer))
                else:
                    raise LexerError(f"unexpected input \"{lexer.current()}\"")
    tokens.append(Token("", TokenType.END_OF_INPUT))
    return tokens


def comment(lexer: "Lexer") -> None:
    assert lexer.current() == "/"
    lexer.next()
    if lexer.current() != "/":
        raise LexerError('expected "/"')
    lexer.next()
    while not lexer.is_end_of_input() and lexer.current() != "\n":
        lexer.next()


def is_valid_identifier_start(c: str) -> bool:
    return c.isalpha()


def is_valid_identifier_char(c: str) -> bool:
    return is_valid_identifier_start(c) or c == "_" or c.isdecimal()


def identifier_or_keyword(lexer: "Lexer") -> Token:
    starting_index = lexer.index
    while not lexer.is_end_of_input() and is_valid_identifier_char(lexer.current()):
        lexer.next()
    contents = lexer.input_[starting_index:lexer.index]

    keywords = {
        "by_move": TokenType.BY_MOVE,
        "type": TokenType.TYPE,
        "function": TokenType.FUNCTION,
        "implement": TokenType.IMPLEMENT,
    }

    if contents not in keywords:
        return Token(contents, TokenType.IDENTIFIER)
    return Token(contents, keywords[contents])


def string_literal(lexer: "Lexer") -> Token:
    assert lexer.current() == "{"
    starting_index = lexer.index
    lexer.next()  # consume "{"
    nesting_level = 1
    while not lexer.is_end_of_input() and not (lexer.current() == "}" and nesting_level == 1):
        if lexer.current() == "{":
            nesting_level += 1
        elif lexer.current() == "}":
            nesting_level -= 1
        lexer.next()
    if lexer.is_end_of_input():
        raise LexerError("unclosed string literal")
    assert lexer.current() == "}"
    lexer.next()  # consume "}"
    contents = lexer.input_[starting_index:lexer.index]
    return Token(contents, TokenType.STRING_LITERAL)


class Lexer:
    def __init__(self, input_: str) -> None:
        self.input_ = input_
        self.index = 0

    def current(self) -> str:
        return self.input_[self.index]

    def peek(self) -> str | None:
        if self.is_end_of_input():
            return None
        return self.input_[self.index + 1]

    def next(self) -> None:
        self.advance(1)

    def advance(self, amount: int) -> None:
        self.index += amount

    def is_end_of_input(self) -> bool:
        return self.index >= len(self.input_)
