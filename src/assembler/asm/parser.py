from typing import Callable

from asm.token import Token, TokenType
import asm.ast as ast


class Parser:
    def __init__(self, tokens: list[Token], on_error: Callable[[int, str], None]):
        self._tokens = tokens
        self._on_error = on_error
        self._current_pos = 0

    # ===================
    #  Statement Parsers
    # ===================

    def parse(self) -> ast.Program:
        statements: list[ast.Statement] = []

        while not self._is_at_end():
            stmt = self._parse_statement()
            statements.append(stmt)

        return ast.Program(statements)

    def _parse_statement(self) -> ast.Statement:
        if (
            self._peek_type() == TokenType.IDENTIFIER
            and self._peek_next_type() == TokenType.COLON
        ):
            return self._parse_label_definition()

        if self._match(TokenType.ADD):
            return self._parse_add()

        raise RuntimeError(f"Expected an instruction or a label, found {self._peek()}")

    def _parse_register(self) -> ast.Register:
        token = self._consume(
            TokenType.IDENTIFIER, "Expected an identifider as register name"
        )
        lexeme = token.lexeme
        if len(lexeme) == 0 or lexeme[0] != "x":
            raise RuntimeError(f"{lexeme} is not a valid register name")

        reg_idx = int(lexeme[1:])
        if reg_idx < 0 or reg_idx > 15:
            raise RuntimeError(f"{reg_idx} is not a valid register idx")

        return ast.Register(reg_idx)

    def _parse_add(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstADD(rd, rs1, rs2)

    def _parse_label_definition(self) -> ast.Statement:
        pass

    # ============
    #  Intrinsics
    # ============

    def _peek(self) -> Token:
        return self._tokens[self._current_pos]

    def _peek_type(self) -> TokenType:
        return self._tokens[self._current_pos].type

    def _peek_next_type(self) -> TokenType:
        return self._tokens[self._current_pos + 1].type

    def _match(self, token_type: TokenType) -> bool:
        if self._peek_type() == token_type:
            self._current_pos += 1
            return True
        return False

    def _consume(self, token_type: TokenType, error_msg: str) -> Token:
        if self._peek_type() == token_type:
            token = self._peek()
            self._current_pos += 1
            return token
        raise RuntimeError(error_msg)

    def _is_at_end(self) -> bool:
        return self._peek_type() == TokenType.EOF
