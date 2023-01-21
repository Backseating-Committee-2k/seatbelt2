use crate::scanner::Token;
use std::{self, collections::HashMap, error::Error, fmt::Display};

#[derive(Debug, Clone)]
pub(crate) enum ParserError {
    UnexpectedToken {
        expected: &'static str,
        actual: Token,
    },
    UnknownConstantName(String),
    EmptyProduction,
    MissingCodeBlock,
    InvalidRuleOrder,
}

impl Error for ParserError {}

impl Display for ParserError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}

#[derive(Debug, Clone)]
pub(crate) enum Symbol {
    Terminal { token: String },
    NonTerminal { symbol: String },
}

impl Display for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Symbol::Terminal { token } => write!(f, "Terminal({token})"),
            Symbol::NonTerminal { symbol } => write!(f, "NonTerminal({symbol})"),
        }
    }
}

struct Parser<'a> {
    pub(crate) tokens: &'a [Token],
    pub(crate) index: usize,
}

pub(crate) enum RuleRhs {
    Proxy {
        symbol: Symbol,
    },
    WithCode {
        symbols: Vec<(Symbol, Option<String>)>,
        code: String,
    },
}

impl Display for RuleRhs {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            RuleRhs::Proxy { symbol } => write!(f, "{symbol}"),
            RuleRhs::WithCode { symbols, code } => symbols
                .iter()
                .fold(Ok(()), |result, symbol| {
                    result.and_then(|_| write!(f, "{}", symbol.0))
                })
                .and_then(|_| write!(f, " {:?}", code)),
        }
    }
}

pub(crate) type Ruleset = Vec<RuleRhs>;

macro_rules! consume {
    ($token_type:pat, $parser:expr) => {
        if matches!($parser.current(), $token_type) {
            let result = $parser.current_cloned();
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
    pub(crate) fn parse(&mut self) -> Result<GeneratorData, ParserError> {
        let constants = self.parse_constants()?;
        let rulesets = self.parse_rulesets()?;
        Ok(GeneratorData {
            constants,
            rulesets,
        })
    }

    pub(crate) fn parse_constants(&mut self) -> Result<HashMap<String, String>, ParserError> {
        let mut constants = HashMap::new();
        while let (Token::Identifier(identifier), Some(Token::Equals)) =
            (self.current(), self.peek())
        {
            let identifier = identifier.clone();
            if !Self::is_valid_constant_name(&identifier) {
                return Err(ParserError::UnknownConstantName(identifier));
            }
            self.next(); // consume identifier
            self.next(); // consume '='
            let Token::StringLiteral(value) = consume!(Token::StringLiteral(_), self)? else { unreachable!() };
            consume!(Token::Semicolon, self)?;
            constants.insert(identifier, value);
        }
        Ok(constants)
    }

    pub(crate) fn parse_rulesets(&mut self) -> Result<HashMap<String, Ruleset>, ParserError> {
        let mut rulesets = HashMap::new();
        let mut current_lhs = None;
        while let (Token::Identifier(identifier), Some(Token::Arrow)) =
            (self.current(), self.peek())
        {
            let identifier = identifier.clone();
            self.next(); // consume identifier
            self.next(); // consume arrow

            if let Some(current_lhs) = &current_lhs {
                if current_lhs != &identifier && rulesets.contains_key(&identifier) {
                    return Err(ParserError::InvalidRuleOrder);
                }
            }

            let entry = rulesets.entry(identifier.clone()).or_insert(Vec::new());

            current_lhs = Some(identifier);

            if let (Token::Identifier(non_terminal), Some(Token::Semicolon)) =
                (self.current(), self.peek())
            {
                let non_terminal = non_terminal.clone();
                self.next(); // consume non-terminal
                self.next(); // consume ';'
                entry.push(RuleRhs::Proxy {
                    symbol: Symbol::NonTerminal {
                        symbol: non_terminal,
                    },
                });
                continue;
            }

            let current_rules = self.parse_multiple_rules_rhs()?;
            let Token::StringLiteral(code_block) = consume!(Token::StringLiteral(_), self)? else {
                unreachable!();
            };
            consume!(Token::Semicolon, self)?;
            for rule in current_rules {
                entry.push(RuleRhs::WithCode {
                    symbols: rule,
                    code: code_block.clone(),
                });
            }
        }
        if !self.is_end_of_input() {
            return Err(ParserError::UnexpectedToken {
                expected: "identifier",
                actual: self.current_cloned(),
            });
        }
        Ok(rulesets)
    }

    pub(crate) fn parse_multiple_rules_rhs(
        &mut self,
    ) -> Result<Vec<Vec<(Symbol, Option<String>)>>, ParserError> {
        let mut rules = Vec::new();
        rules.push(self.parse_single_rule_rhs()?);
        while matches!(self.current(), Token::Pipe) {
            self.next(); // consume '|'
            rules.push(self.parse_single_rule_rhs()?);
        }
        Ok(rules)
    }

    pub(crate) fn parse_single_rule_rhs(
        &mut self,
    ) -> Result<Vec<(Symbol, Option<String>)>, ParserError> {
        let mut symbols = Vec::new();
        loop {
            if let Token::Identifier(identifier) = self.current_cloned() {
                self.next(); // consume identifier
                symbols.push((
                    Symbol::NonTerminal { symbol: identifier },
                    if matches!(self.current(), Token::Colon) {
                        self.next(); // consume ':'
                        let Token::Identifier(alias) = consume!(Token::Identifier(_), self)? else {
                            unreachable!();
                        };
                        Some(alias)
                    } else {
                        None
                    },
                ));
                continue;
            }

            if let Token::TokenLiteral(token) = self.current_cloned() {
                self.next(); // consume token literal
                symbols.push((
                    Symbol::Terminal { token },
                    if matches!(self.current(), Token::Colon) {
                        self.next(); // consume ':'
                        let Token::Identifier(alias) = consume!(Token::Identifier(_), self)? else {
                            unreachable!();
                        };
                        Some(alias)
                    } else {
                        None
                    },
                ));
                continue;
            }

            break;
        }
        Ok(symbols)
    }

    pub(crate) fn is_end_of_input(&self) -> bool {
        self.index >= self.tokens.len() || matches!(self.current(), Token::EndOfInput)
    }

    pub(crate) fn current(&self) -> &Token {
        &self.tokens[self.index]
    }

    pub(crate) fn current_cloned(&self) -> Token {
        self.current().clone()
    }

    pub(crate) fn next(&mut self) {
        self.index += 1;
    }

    pub(crate) fn peek(&self) -> Option<&Token> {
        if self.is_end_of_input() {
            None
        } else {
            Some(&self.tokens[self.index + 1])
        }
    }

    pub(crate) fn is_valid_constant_name(name: &str) -> bool {
        ["DEFAULT_RETURN_VALUE", "GLOBAL_CODE"].contains(&name)
    }
}

pub(crate) struct GeneratorData {
    pub(crate) constants: HashMap<String, String>,
    pub(crate) rulesets: HashMap<String, Ruleset>,
}

impl GeneratorData {
    pub(crate) fn from_tokens(tokens: &[Token]) -> Result<Self, ParserError> {
        let mut parser = Parser { tokens, index: 0 };
        parser.parse()
    }
}
