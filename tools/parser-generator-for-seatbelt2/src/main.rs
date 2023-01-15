use std::{error::Error, path::PathBuf};

use clap::Parser;

mod scanner;

#[derive(Parser)]
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
    Ok(())
}
