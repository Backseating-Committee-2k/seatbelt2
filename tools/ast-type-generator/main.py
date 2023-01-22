import sys

from lexer import tokenize, LexerError
from parser import parse, ParserError


def main():
    if len(sys.argv) != 2:
        print("specify exactly one source file", file=sys.stderr)
        sys.exit(1)
    filename = sys.argv[1]
    with open(filename) as file:
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


if __name__ == "__main__":
    main()
