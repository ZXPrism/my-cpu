import argparse

from asm.tokenizer import Tokenizer
from asm.parser import Parser
from asm.codegen import Codegen

from rich import print


def error(line_no: int, msg: str):
    print(f"[line {line_no}] Error: {msg}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", "-i", dest="input", required=True)
    parser.add_argument("--output", "-o", dest="output", required=True)
    parser.add_argument("--tokens", action="store_true")
    parser.add_argument("--ast", action="store_true")
    args = parser.parse_args()

    with open(args.input, "r") as fp:
        src = fp.read()

    tokenizer = Tokenizer(src, lambda line_no, msg: error(line_no, msg))
    tokens = tokenizer.scan_tokens()
    if args.tokens:
        for token in tokens:
            print(token)

    parser = Parser(tokens)
    program = parser.parse()
    if args.ast:
        print(program)

    codegen = Codegen(program)
    codegen.emit(args.output)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
