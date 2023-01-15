use std::{collections::HashMap, error::Error, fmt::Display, path::PathBuf};

use scanner::Token;

mod scanner;

#[derive(clap::Parser)]
struct CommandLineArguments {
    source_filename: PathBuf,
    #[arg(short = 'o', long = "out")]
    destination_filename: Option<PathBuf>,
}

#[derive(Debug, Clone)]
enum ParserError {
    UnexpectedToken {
        expected: &'static str,
        actual: Token,
    },
    UnknownConstantName(String),
}

impl Error for ParserError {}

impl Display for ParserError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}

enum ParserState {
    Constants,
    Rules,
}

enum Symbol {
    Terminal { token: String },
    NonTerminal { symbol: String },
}

struct Parser<'a> {
    state: ParserState,
    tokens: &'a [Token],
    index: usize,
}

struct ProductionRhs {
    symbols: Vec<(Symbol, Option<String>)>,
}

type Ruleset = Vec<ProductionRhs>;

macro_rules! consume {
    ($token_type:pat, $parser:expr) => {
        if matches!($parser.current(), $token_type) {
            let result = $parser.current().clone();
            $parser.next();
            Ok(result)
        } else {
            Err(ParserError::UnexpectedToken {
                expected: stringify!($token_type),
                actual: $parser.current().clone(),
            })
        }
    };
}

impl Parser<'_> {
    fn parse(&mut self) -> Result<GeneratorData, ParserError> {
        let mut constants = HashMap::new();
        let mut rulesets = HashMap::new();
        while !self.is_end_of_input() {
            match (&self.state, self.current(), self.peek()) {
                (ParserState::Constants, Token::Identifier(name), Some(Token::Equals)) => {
                    let constant_name = name.clone();
                    if !Self::is_valid_constant_name(&constant_name) {
                        return Err(ParserError::UnknownConstantName(constant_name));
                    }
                    self.next(); // consume the constant name
                    consume!(Token::Equals, self)?;
                    let Token::StringLiteral(constant_value) =
                        consume!(Token::StringLiteral(_), self)? else { unreachable!()};
                    consume!(Token::Semicolon, self)?;
                    constants.insert(constant_name, constant_value);
                }
                (ParserState::Constants, Token::Identifier(_), _) => {
                    self.state = ParserState::Rules;
                }
                (ParserState::Constants, actual, _) => {
                    return Err(ParserError::UnexpectedToken {
                        expected: "identifier",
                        actual: actual.clone(),
                    })
                }
                (ParserState::Rules, _, _) => todo!(),
            }
        }
        Ok(GeneratorData {
            constants,
            rulesets,
        })
    }

    fn is_end_of_input(&self) -> bool {
        self.index >= self.tokens.len() || matches!(self.current(), Token::EndOfInput)
    }

    fn current(&self) -> &Token {
        &self.tokens[self.index]
    }

    fn next(&mut self) {
        self.index += 1;
    }

    fn peek(&self) -> Option<&Token> {
        if self.is_end_of_input() {
            None
        } else {
            Some(&self.tokens[self.index + 1])
        }
    }

    fn is_valid_constant_name(name: &str) -> bool {
        ["DEFAULT_RETURN_VALUE", "GLOBAL_CODE"].contains(&name)
    }
}

struct GeneratorData {
    constants: HashMap<String, String>,
    rulesets: HashMap<String, Ruleset>,
}

impl GeneratorData {
    pub(crate) fn from_tokens(tokens: &[Token]) -> Result<Self, ParserError> {
        let mut parser = Parser {
            state: ParserState::Constants,
            tokens,
            index: 0,
        };
        parser.parse()
    }
}

fn main() -> Result<(), Box<dyn Error>> {
    use clap::Parser as _;
    let command_line_arguments = CommandLineArguments::parse();
    let input = std::fs::read_to_string(command_line_arguments.source_filename)
        .expect("unable to read input file");
    let tokens = scanner::tokenize(&input)?;
    let generator_data = GeneratorData::from_tokens(&tokens)?;
    for (name, value) in &generator_data.constants {
        println!("{name}: {value}");
    }
    Ok(())
}
