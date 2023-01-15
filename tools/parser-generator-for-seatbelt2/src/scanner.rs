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
                b':' => {
                    scanner.next();
                    tokens.push(Token::Colon);
                }
                b'|' => {
                    scanner.next();
                    tokens.push(Token::Pipe);
                }
                b'=' => {
                    scanner.next();
                    tokens.push(Token::Equals);
                }
                b'-' => {
                    scanner.next();
                    scanner.consume(b'>')?;
                    tokens.push(Token::Arrow);
                }
                b'/' => {
                    scanner.next();
                    scanner.consume(b'/')?;
                    while !scanner.is_end_of_input() && scanner.current() != b'\n' {
                        scanner.next();
                    }
                }
                b'"' => {
                    scanner.next();
                    let starting_index = scanner.index;
                    while !scanner.is_end_of_input() && scanner.current() != b'"' {
                        if char::is_whitespace(scanner.current() as char) {
                            return Err(TokenizerError::WhitespaceInTokenLiteral(
                                scanner.substring_until_current(starting_index),
                            ));
                        }
                        scanner.next();
                    }
                    if scanner.is_end_of_input() {
                        return Err(TokenizerError::UnclosedTokenLiteral(
                            scanner.substring_until_current(starting_index),
                        ));
                    }
                    let contents = scanner.substring_until_current(starting_index);
                    scanner.next(); // consume closing '"'
                    tokens.push(Token::TokenLiteral(contents));
                }
                b'{' => {
                    scanner.next();
                    let starting_index = scanner.index;
                    let mut nesting_level = 1_u32;

                    #[allow(clippy::nonminimal_bool)]
                    while !scanner.is_end_of_input()
                        && !(nesting_level == 1 && scanner.current() == b'}')
                    {
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
                }
                current if is_identifier_start(current) => {
                    let starting_index = scanner.index;
                    scanner.next();
                    while !scanner.is_end_of_input() && is_identifier_character(scanner.current()) {
                        scanner.next();
                    }
                    tokens.push(Token::Identifier(
                        scanner.substring_until_current(starting_index),
                    ));
                }
                current if char::is_whitespace(current as char) => {
                    scanner.next();
                }
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

pub(crate) fn tokenize(input: &str) -> Result<Vec<Token>, TokenizerError> {
    input
        .is_ascii()
        .then_some(input)
        .ok_or(TokenizerError::InvalidCharacters)
        .and_then(|input| Scanner::tokenize(input.as_bytes()))
}
