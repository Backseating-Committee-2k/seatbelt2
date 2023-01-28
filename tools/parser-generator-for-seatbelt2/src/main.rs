use crate::parser::GeneratorData;
use clap::Parser as _;
use std::{error::Error, path::PathBuf};

mod parser;
mod scanner;

#[derive(clap::Parser)]
struct CommandLineArguments {
    source_filename: PathBuf,
    #[arg(short = 'o', long = "out")]
    destination_filename: Option<PathBuf>,
}

fn main() -> Result<(), Box<dyn Error>> {
    let command_line_arguments = CommandLineArguments::parse();
    let input = std::fs::read_to_string(command_line_arguments.source_filename)
        .expect("unable to read input file");
    let tokens = scanner::tokenize(&input)?;
    let generator_data = GeneratorData::from_tokens(&tokens)?;
    for (name, value) in &generator_data.constants {
        println!("{name}: {value}");
    }
    for (lhs, rhs) in &generator_data.rulesets {
        println!("{lhs}");
        for production in rhs {
            println!("\t{}", production);
        }
    }
    Ok(())
}
