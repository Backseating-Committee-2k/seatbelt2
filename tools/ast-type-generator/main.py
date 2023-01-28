import sys

from emitter import emit
from lexer import tokenize, LexerError
from parser import parse, ParserError


def main() -> None:
    if len(sys.argv) != 4:
        print("specify exactly one source file, the base filename for the output files, and a namespace name",
              file=sys.stderr)
        sys.exit(1)
    input_filename = sys.argv[1]
    base_filename = sys.argv[2]
    namespace_name = sys.argv[3]
    with open(input_filename) as file:
        contents = file.read()
    try:
        tokens = tokenize(contents)
    except LexerError as exception:
        print(f"lexer error: {exception}", file=sys.stderr)
        sys.exit(1)

    print(f"{len(tokens)} tokens found")
    print("\n".join(str(token) for token in tokens))

    try:
        generator_description = parse(tokens)
    except ParserError as exception:
        print(f"parser error: {exception}", file=sys.stderr)
        sys.exit(1)

    print(generator_description)

    try:
        emit(input_filename, generator_description, base_filename, namespace_name)
    except OSError as exception:
        print(f"unable to write files: {exception}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
