use std::{error::Error, fmt::Display};

#[derive(Debug)]
pub(crate) enum Token {
    Identifier(String),
    StringLiteral(String),
    TokenLiteral(String), // for example "LEFT_PARENTHESIS" or "SEMICOLON"
    Arrow,
    Pipe,
    Colon,
    Equals,
}

#[derive(Debug, Clone)]
pub(crate) enum TokenizerError {
    InvalidCharacters,
    UnexpectedCharacter { expected: Option<u8>, actual: u8 },
    UnclosedStringLiteral(String),
    UnclosedTokenLiteral(String),
    WhitespaceInTokenLiteral(String),
}

impl Error for TokenizerError {}

impl Display for TokenizerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}

fn is_identifier_start(character: u8) -> bool {
    (b'a'..=b'z').contains(&character) || (b'A'..=b'Z').contains(&character)
}

fn is_identifier_character(character: u8) -> bool {
    is_identifier_start(character) || (b'0'..=b'9').contains(&character) || character == b'_'
}

struct Scanner<'a> {
    input: &'a [u8],
    index: usize,
}

impl<'a> Scanner<'a> {
    fn tokenize(input: &'a [u8]) -> Result<Vec<Token>, TokenizerError> {
        let mut scanner = Scanner { input, index: 0 };
        let mut tokens = Vec::new();
        while !scanner.is_end_of_input() {
            match scanner.current() {
                b':' => consume_into_token(&mut scanner, &mut tokens, Token::Colon),
                b'|' => consume_into_token(&mut scanner, &mut tokens, Token::Pipe),
                b'=' => consume_into_token(&mut scanner, &mut tokens, Token::Equals),
                b'-' => arrow(&mut scanner, &mut tokens)?,
                b'/' => comment(&mut scanner)?,
                b'"' => token_literal(&mut scanner, &mut tokens)?,
                b'{' => string_literal(&mut scanner, &mut tokens)?,
                current if is_identifier_start(current) => identifier(&mut scanner, &mut tokens)?,
                current if char::is_whitespace(current as char) => scanner.next(),
                current => {
                    return Err(TokenizerError::UnexpectedCharacter {
                        expected: None,
                        actual: current,
                    })
                }
            }
        }
        Ok(tokens)
    }

    fn is_end_of_input(&self) -> bool {
        self.index >= self.input.len()
    }

    fn current(&self) -> u8 {
        self.input[self.index]
    }

    fn next(&mut self) {
        self.advance(1)
    }

    fn advance(&mut self, amount: usize) {
        self.index += amount
    }

    fn consume(&mut self, expected: u8) -> Result<(), TokenizerError> {
        if self.is_end_of_input() || self.current() != expected {
            return Err(TokenizerError::UnexpectedCharacter {
                expected: Some(expected),
                actual: self.current(),
            });
        }
        self.next();
        Ok(())
    }

    fn substring_until_current(&self, starting_index: usize) -> String {
        std::str::from_utf8(&self.input[starting_index..self.index])
            .unwrap()
            .to_string()
    }
}

fn identifier(scanner: &mut Scanner, tokens: &mut Vec<Token>) -> Result<(), TokenizerError> {
    let starting_index = scanner.index;
    scanner.next();

    loop {
        if scanner.is_end_of_input() || !is_identifier_character(scanner.current()) {
            break;
        }
        scanner.next();
    }

    tokens.push(Token::Identifier(
        scanner.substring_until_current(starting_index),
    ));
    Ok(())
}

fn string_literal(scanner: &mut Scanner, tokens: &mut Vec<Token>) -> Result<(), TokenizerError> {
    scanner.next(); // consume the leading "{"
    let starting_index = scanner.index;
    let mut nesting_level = 1_u32;

    loop {
        if scanner.is_end_of_input() || (nesting_level == 1 && scanner.current() == b'}') {
            break;
        }
        match scanner.current() {
            b'{' => nesting_level += 1,
            b'}' => nesting_level -= 1,
            _ => {}
        }
        scanner.next();
    }

    if scanner.is_end_of_input() {
        return Err(TokenizerError::UnclosedStringLiteral(
            scanner.substring_until_current(starting_index),
        ));
    }

    let contents = scanner.substring_until_current(starting_index);
    scanner.next(); // consume closing '}'
    tokens.push(Token::StringLiteral(contents.trim().to_string()));
    Ok(())
}

fn token_literal(scanner: &mut Scanner, tokens: &mut Vec<Token>) -> Result<(), TokenizerError> {
    scanner.next(); // consume leading '"'
    let starting_index = scanner.index;

    loop {
        if scanner.is_end_of_input() || scanner.current() == b'"' {
            break;
        }
        scanner.next();
    }

    let contents = scanner.substring_until_current(starting_index);

    if contents.contains(|char: char| char.is_ascii_whitespace()) {
        return Err(TokenizerError::WhitespaceInTokenLiteral(contents));
    }

    if scanner.is_end_of_input() {
        return Err(TokenizerError::UnclosedTokenLiteral(contents));
    }

    scanner.next(); // consume closing '"'
    tokens.push(Token::TokenLiteral(contents));
    Ok(())
}

fn comment(scanner: &mut Scanner) -> Result<(), TokenizerError> {
    scanner.next();
    scanner.consume(b'/')?;
    while !scanner.is_end_of_input() && scanner.current() != b'\n' {
        scanner.next();
    }
    Ok(())
}

fn arrow(scanner: &mut Scanner, tokens: &mut Vec<Token>) -> Result<(), TokenizerError> {
    scanner.next();
    scanner.consume(b'>')?;
    tokens.push(Token::Arrow);
    Ok(())
}

fn consume_into_token(scanner: &mut Scanner, tokens: &mut Vec<Token>, token: Token) {
    scanner.next();
    tokens.push(token);
}

pub(crate) fn tokenize(input: &str) -> Result<Vec<Token>, TokenizerError> {
    input
        .is_ascii()
        .then_some(input)
        .ok_or(TokenizerError::InvalidCharacters)
        .and_then(|input| Scanner::tokenize(input.as_bytes()))
}
