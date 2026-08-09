from asm.tokenizer import Tokenizer
from asm.parser import Parser


def error(line_no: int, msg: str):
    print(f"[line {line_no}] Error: {msg}")


def main():
    with open("asm/test/test.s", "r") as fp:
        src = fp.read()

    tokenizer = Tokenizer(src, lambda line_no, msg: error(line_no, msg))
    tokens = tokenizer.scan_tokens()
    for token in tokens:
        print(token)

    parser = Parser(tokens, lambda line_no, msg: error(line_no, msg))
    program = parser.parse()
    print(program)


if __name__ == "__main__":
    main()
