# P.S. tokenizer is just lexer here
# I use tokenizer because it sounds coolder :)

from typing import Callable

from asm.token import Token, TokenType

_map_keyword_to_token_type = {
    "ADD": TokenType.ADD,
    "SUB": TokenType.SUB,
    "SLL": TokenType.SLL,
    "SRL": TokenType.SRL,
    "SRA": TokenType.SRA,
    "SLT": TokenType.SLT,
    "SLTU": TokenType.SLTU,
    "XOR": TokenType.XOR,
    "OR": TokenType.OR,
    "AND": TokenType.AND,
    "STR": TokenType.STR,
    "LDR": TokenType.LDR,
    "LI": TokenType.LI,
    "JAL": TokenType.JAL,
    "BAL": TokenType.BAL,
    "INT": TokenType.INT,
    "CLR": TokenType.CLR,
    "DB": TokenType.DB,
    "HLT": TokenType.HLT,
    "JMP": TokenType.JMP,
    "LL": TokenType.LL,
    "MOV": TokenType.MOV,
}


class Tokenizer:
    def __init__(self, src: str, on_error: Callable[[int, str], None]):
        self._source = src
        self._on_error = on_error

        self._tokens: list[Token] = []
        self._start_pos = 0
        self._current_pos = 0
        self._line_no = 1

    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self._start_pos = self._current_pos
            self._scan_token()

        self._tokens.append(
            Token(
                type=TokenType.EOF,
                lexeme="",
                literal=None,
                line_no=self._line_no,
            )
        )

        return self._tokens

    def _scan_token(self):
        ch = self._advance()
        match ch:
            case ":":
                self._add_token(TokenType.COLON, None)
            case ",":
                self._add_token(TokenType.COMMA, None)
            case ";":
                while self._peek() != "\n" and not self._is_at_end():
                    self._advance()
            case '"':
                self._scan_string_literal()
            case "\n":
                self._line_no += 1
            case " " | "\r" | "\t":
                pass
            case _:
                if self._is_digit(ch):
                    self._scan_number_literal()
                elif self._is_alpha(ch):
                    self._scan_identifier()
                else:
                    self._on_error(self._line_no, f"Unexpected character: {ch}")

    def _is_alpha(self, ch: str):
        if len(ch) != 1:
            return False
        return (
            ord("a") <= ord(ch) <= ord("z")
            or ord("A") <= ord(ch) <= ord("Z")
            or ch == "_"
        )

    def _is_alpha_numeric(self, ch: str):
        return self._is_alpha(ch) or self._is_digit(ch)

    def _is_digit(self, ch: str):
        if len(ch) != 1:
            return False
        return ord("0") <= ord(ch) <= ord("9")

    def _scan_identifier(self):
        while self._is_alpha_numeric(self._peek()):
            self._advance()
        lexeme = self._source[self._start_pos : self._current_pos]
        if lexeme in _map_keyword_to_token_type:
            self._add_token(_map_keyword_to_token_type[lexeme], None)
        else:
            self._add_token(TokenType.IDENTIFIER, None)

    def _scan_number_literal(self):
        while self._is_digit(self._peek()):
            self._advance()

        self._add_token(
            TokenType.NUMBER, int(self._source[self._start_pos : self._current_pos])
        )

    def _peek_next(self):
        if self._current_pos + 1 >= len(self._source):
            return "\0"
        return self._source[self._current_pos + 1]

    def _scan_string_literal(self):
        while self._peek() != '"':
            if self._peek() == "\n":
                self._line_no += 1
            self._advance()
        if self._is_at_end():
            self._on_error(self._line_no, "Unterminated string")
            return
        self._advance()
        literal = self._source[self._start_pos + 1 : self._current_pos - 1]
        self._add_token(TokenType.STRING, literal)

    def _peek(self) -> str:
        if self._is_at_end():
            return "\0"
        return self._source[self._current_pos]

    def _advance(self) -> str:
        ch = self._source[self._current_pos]
        self._current_pos += 1
        return ch

    def _match(self, expected: str) -> bool:
        if self._is_at_end() or self._source[self._current_pos] != expected:
            return False
        self._current_pos += 1
        return True

    def _add_token(self, token_type: TokenType, literal: object | None):
        lexeme = self._source[self._start_pos : self._current_pos]
        self._tokens.append(
            Token(
                type=token_type,
                lexeme=lexeme,
                literal=literal,
                line_no=self._line_no,
            )
        )

    def _is_at_end(self) -> bool:
        return self._current_pos >= len(self._source)
