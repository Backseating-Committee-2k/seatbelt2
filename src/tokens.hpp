#pragma once

#include "source_location.hpp"
#include "types.hpp"
#include <string_view>
#include <variant>
#include <vector>

enum class TokenType {
    And,
    Arrow,
    Asterisk,
    At,
    BoolLiteral,
    Break,
    CapitalizedFunction,
    CharLiteral,
    Colon,
    Comma,
    Const,
    Continue,
    Do,
    Dot,
    DoubleColon,
    DumpRegisters,
    Else,
    EndOfFile,
    Equals,
    EqualsEquals,
    ExclamationMark,
    ExclamationMarkEquals,
    Export,
    For,
    ForwardSlash,
    Function,
    Goto,
    GreaterOrEquals,
    GreaterThan,
    Identifier,
    If,
    Import,
    InlineBssembly,
    Label,
    LeftCurlyBracket,
    LeftParenthesis,
    LeftSquareBracket,
    LessOrEquals,
    LessThan,
    Let,
    Loop,
    Minus,
    Mod,
    Mutable,
    Namespace,
    Not,
    NothingLiteral,
    Or,
    Plus,
    Restricted,
    Return,
    RightCurlyBracket,
    RightParenthesis,
    RightSquareBracket,
    Semicolon,
    Struct,
    TildeArrow,
    Type,
    TypeSize,
    U32Literal,
    ValueSize,
    While,
    Xor,
};

struct Token {
    Token(const SourceLocation location, const TokenType type) : location{ location }, type{ type } { }

    SourceLocation location;
    TokenType type;
};
