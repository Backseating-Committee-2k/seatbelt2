use std::path::PathBuf;

use clap::Parser;

mod scanner;

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
    match scanner::tokenize(&input) {
        Ok(tokens) => {
            for token in tokens {
                println!("{token:?}");
            }
        }
        Err(error) => eprintln!("{error:?}"),
    }
}
