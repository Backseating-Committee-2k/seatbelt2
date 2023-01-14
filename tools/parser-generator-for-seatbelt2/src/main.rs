use std::path::PathBuf;

use clap::Parser;

#[derive(Debug)]
enum Token {
    Identifier(String),
    StringLiteral(String),
    TokenLiteral(String), // for example "LEFT_PARENTHESIS" or "SEMICOLON"
    Arrow,
    Pipe,
    Colon,
    Equals,
}

#[derive(Debug, Clone)]
enum TokenizerError {
    InvalidCharacters,
    UnexpectedCharacter { expected: Option<u8>, actual: u8 },
    UnexpectedEndOfInput,
    UnclosedStringLiteral(String),
    UnclosedTokenLiteral(String),
    WhitespaceInTokenLiteral(String),
}

#[derive(Debug)]
enum LexerState {
    Normal,
    Identifier(String),
    Arrow,
    StringLiteral {
        contents: String,
        nesting_depth: u32,
    },
    TokenLiteral(String),
    Comment {
        num_slashes: u8,
    },
}

struct LexerData<'a> {
    tokens: Vec<Token>,
    input: &'a [u8],
    state: LexerState,
}

impl<'a> LexerData<'a> {
    pub fn new(tokens: Vec<Token>, input: &'a [u8], state: LexerState) -> Self {
        LexerData {
            tokens,
            input,
            state,
        }
    }
}

fn is_identifier_start(character: u8) -> bool {
    (b'a'..=b'z').contains(&character) || (b'A'..=b'Z').contains(&character)
}

fn is_identifier_character(character: u8) -> bool {
    is_identifier_start(character) || (b'0'..=b'9').contains(&character) || character == b'_'
}

fn proceed(lexer_data: LexerData) -> Result<LexerData, TokenizerError> {
    use LexerState::*;
    use TokenizerError::*;

    let mut tokens = lexer_data.tokens;
    let state = lexer_data.state;
    let input = lexer_data.input;
    let current_char = (!input.is_empty()).then(|| input[0]);

    match (&state, current_char) {
        (Normal, None) => Ok(LexerData::new(tokens, input, state)),
        (Normal, Some(actual)) if char::is_whitespace(actual as char) => {
            proceed(LexerData::new(tokens, &input[1..], Normal))
        }
        (Normal, Some(b':')) => {
            let mut tokens = tokens;
            let token = Token::Colon;
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (Normal, Some(b'=')) => {
            let mut tokens = tokens;
            let token = Token::Equals;
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (Normal, Some(b'|')) => {
            let mut tokens = tokens;
            let token = Token::Pipe;
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (Normal, Some(b'-')) => proceed(LexerData::new(tokens, &input[1..], Arrow)),
        (Normal, Some(b'/')) => {
            let next_state = Comment { num_slashes: 1 };
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
        (Normal, Some(actual)) if is_identifier_start(actual) => {
            let next_state = Identifier(String::from(actual as char));
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
        (Normal, Some(b'{')) => {
            let next_state = StringLiteral {
                contents: String::new(),
                nesting_depth: 1,
            };
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
        (Normal, Some(b'"')) => {
            let next_state = TokenLiteral(String::new());
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
        (Normal, Some(actual)) => Err(UnexpectedCharacter {
            expected: None,
            actual,
        }),
        (Identifier(contents), Some(actual)) if is_identifier_character(actual) => {
            let mut contents = contents.clone();
            contents.push(actual as char);
            {
                let next_state = Identifier(contents);
                proceed(LexerData::new(tokens, &input[1..], next_state))
            }
        }
        (Identifier(contents), _) => {
            tokens.push(Token::Identifier(contents.clone()));
            proceed(LexerData::new(tokens, input, Normal))
        }
        (Arrow, Some(b'>')) => {
            let mut tokens = tokens;
            let token = Token::Arrow;
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (Arrow, Some(actual)) => Err(UnexpectedCharacter {
            expected: Some(b'>'),
            actual,
        }),
        (Arrow, None) => Err(UnexpectedEndOfInput),
        (StringLiteral { contents, .. }, None) => Err(UnclosedStringLiteral(contents.clone())),
        (
            StringLiteral {
                contents,
                nesting_depth: 1,
            },
            Some(b'}'),
        ) => {
            let mut tokens = tokens;
            let token = Token::StringLiteral(contents.trim().to_string());
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (
            StringLiteral {
                contents,
                nesting_depth,
            },
            Some(b'}'),
        ) => {
            let mut contents = contents.clone();
            contents.push('}');
            {
                let next_state = LexerState::StringLiteral {
                    contents,
                    nesting_depth: *nesting_depth - 1,
                };
                proceed(LexerData::new(tokens, &input[1..], next_state))
            }
        }
        (
            StringLiteral {
                contents,
                nesting_depth,
            },
            Some(b'{'),
        ) => {
            let mut contents = contents.clone();
            contents.push('{');
            {
                let next_state = LexerState::StringLiteral {
                    contents,
                    nesting_depth: *nesting_depth + 1,
                };
                proceed(LexerData::new(tokens, &input[1..], next_state))
            }
        }
        (
            StringLiteral {
                contents,
                nesting_depth,
            },
            Some(actual),
        ) => {
            let mut contents = contents.clone();
            contents.push(actual as char);
            {
                let next_state = LexerState::StringLiteral {
                    contents,
                    nesting_depth: *nesting_depth,
                };
                proceed(LexerData::new(tokens, &input[1..], next_state))
            }
        }
        (TokenLiteral(contents), None) => Err(UnclosedTokenLiteral(contents.clone())),
        (TokenLiteral(contents), Some(b'"')) => {
            let mut tokens = tokens;
            let token = Token::TokenLiteral(contents.clone());
            tokens.push(token);
            proceed(LexerData::new(tokens, &input[1..], LexerState::Normal))
        }
        (TokenLiteral(contents), Some(actual)) if char::is_whitespace(actual as char) => {
            Err(WhitespaceInTokenLiteral(contents.clone()))
        }
        (TokenLiteral(contents), Some(actual)) => {
            let mut contents = contents.clone();
            contents.push(actual as char);
            {
                let next_state = LexerState::TokenLiteral(contents);
                proceed(LexerData::new(tokens, &input[1..], next_state))
            }
        }
        (Comment { num_slashes: 1 }, Some(b'/')) => {
            let next_state = Comment { num_slashes: 2 };
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
        (Comment { num_slashes: 1 }, Some(actual)) => Err(UnexpectedCharacter {
            expected: Some(b'/'),
            actual,
        }),
        (Comment { num_slashes: 1 }, None) => Err(UnexpectedEndOfInput),
        (Comment { .. }, Some(b'\n') | None) => {
            proceed(LexerData::new(tokens, &input[1..], Normal))
        }
        (Comment { .. }, Some(_)) => {
            let next_state = Comment { num_slashes: 2 };
            proceed(LexerData::new(tokens, &input[1..], next_state))
        }
    }
}

fn tokenize(input: &str) -> Result<Vec<Token>, TokenizerError> {
    input
        .is_ascii()
        .then_some(input)
        .ok_or(TokenizerError::InvalidCharacters)
        .map(|input| LexerData::new(Vec::new(), input.as_bytes(), LexerState::Normal))
        .and_then(proceed)
        .map(|lexer_state| lexer_state.tokens)
}

#[derive(Parser)]
struct CommandLineArguments {
    source_filename: PathBuf,
    #[arg(short = 'o', long = "out")]
    destination_filename: Option<PathBuf>,
}

fn main() {
    let command_line_arguments = CommandLineArguments::parse();
    let input = std::fs::read_to_string(command_line_arguments.source_filename)
        .expect("unable to read input file");
    match tokenize(&input) {
        Ok(tokens) => {
            for token in tokens {
                println!("{token:?}");
            }
        }
        Err(error) => eprintln!("{error:?}"),
    }
}
