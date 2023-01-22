from enum import Enum


class LexerError(Exception):
    pass


class TokenType(Enum):
    Type = 0
    Identifier = 1
    StringLiteral = 2
    Equals = 3
    LeftParenthesis = 4
    RightParenthesis = 5
    Pipe = 6
    ByMove = 7
    EndOfInput = 8


class Token:
    def __init__(self, lexeme: str, type_: TokenType) -> None:
        self.lexeme = lexeme
        self.type_ = type_

    def __str__(self):
        return f"{self.type_}(\"{self.lexeme}\")"


def tokenize(input_: str) -> list[Token]:
    lexer = Lexer(input_)
    tokens = list()
    while not lexer.is_end_of_input():
        match lexer.current():
            case "(":
                tokens.append(Token(lexer.current(), TokenType.LeftParenthesis))
                lexer.next()
            case ")":
                tokens.append(Token(lexer.current(), TokenType.RightParenthesis))
                lexer.next()
            case "=":
                tokens.append(Token(lexer.current(), TokenType.Equals))
                lexer.next()
            case "|":
                tokens.append(Token(lexer.current(), TokenType.Pipe))
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
    tokens.append(Token("", TokenType.EndOfInput))
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
    if contents == "by_move":
        return Token(contents, TokenType.ByMove)
    if contents == "type":
        return Token(contents, TokenType.Type)
    return Token(contents, TokenType.Identifier)


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
    return Token(contents, TokenType.StringLiteral)


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
