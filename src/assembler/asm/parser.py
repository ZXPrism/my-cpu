from typing import Callable, cast

from asm.token import Token, TokenType
import asm.ast as ast

from rich import print


class Parser:
    def __init__(self, tokens: list[Token], on_error: Callable[[int, str], None]):
        self._tokens = tokens
        self._on_error = on_error
        self._current_pos = 0
        self._line_no = 0

    # ===================
    #  Statement Parsers
    # ===================

    def parse(self) -> ast.Program:
        statements: list[ast.Statement] = []

        try:
            while not self._is_at_end():
                stmt = self._parse_statement()
                statements.append(stmt)
        except RuntimeError as e:
            print(f"Error at line {self._line_no}: {e}")
            raise e

        return ast.Program(statements)

    def _parse_statement(self) -> ast.Statement:
        if (
            self._peek_type() == TokenType.IDENTIFIER
            and self._peek_next_type() == TokenType.COLON
        ):
            return self._parse_label_definition()

        if self._match(TokenType.ADD):
            return self._parse_add()
        if self._match(TokenType.SUB):
            return self._parse_sub()
        if self._match(TokenType.SLL):
            return self._parse_sll()
        if self._match(TokenType.SRL):
            return self._parse_srl()
        if self._match(TokenType.SRA):
            return self._parse_sra()
        if self._match(TokenType.SLT):
            return self._parse_slt()
        if self._match(TokenType.SLTU):
            return self._parse_sltu()
        if self._match(TokenType.XOR):
            return self._parse_xor()
        if self._match(TokenType.OR):
            return self._parse_or()
        if self._match(TokenType.AND):
            return self._parse_and()
        if self._match(TokenType.STR):
            return self._parse_str()
        if self._match(TokenType.LDR):
            return self._parse_ldr()
        if self._match(TokenType.LI):
            return self._parse_li()
        if self._match(TokenType.JAL):
            return self._parse_jal()
        if self._match(TokenType.BAL):
            return self._parse_bal()
        if self._match(TokenType.INT):
            return self._parse_int()
        if self._match(TokenType.CLR):
            return self._parse_clr()
        if self._match(TokenType.DB):
            return self._parse_db()
        if self._match(TokenType.HLT):
            return self._parse_hlt()
        if self._match(TokenType.JMP):
            return self._parse_jmp()
        if self._match(TokenType.LL):
            return self._parse_ll()
        if self._match(TokenType.MOV):
            return self._parse_mov()

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

    def _parse_sub(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstSUB(rd, rs1, rs2)

    def _parse_sll(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after source register")

        r_shift = self._parse_register()

        return ast.InstSLL(rd, rs, r_shift)

    def _parse_srl(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after source register")

        r_shift = self._parse_register()

        return ast.InstSRL(rd, rs, r_shift)

    def _parse_sra(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after source register")

        r_shift = self._parse_register()

        return ast.InstSRA(rd, rs, r_shift)

    def _parse_slt(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstSLT(rd, rs1, rs2)

    def _parse_sltu(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstSLTU(rd, rs1, rs2)

    def _parse_xor(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstXOR(rd, rs1, rs2)

    def _parse_or(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstOR(rd, rs1, rs2)

    def _parse_and(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rs1 = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after first source register")

        rs2 = self._parse_register()

        return ast.InstAND(rd, rs1, rs2)

    def _parse_str(self) -> ast.Statement:
        rs = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after source register")
        rd_addr = self._parse_register()

        return ast.InstSTR(rs, rd_addr)

    def _parse_ldr(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")
        rs_addr = self._parse_register()

        return ast.InstLDR(rd, rs_addr)

    def _parse_li(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")
        imm = self._consume(TokenType.NUMBER, "Expected an immediate value")

        return ast.InstLI(rd, cast(int, imm.literal))

    def _parse_jal(self) -> ast.Statement:
        r_link = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after link register")
        r_jmp_target = self._parse_register()

        return ast.InstJAL(r_link, r_jmp_target)

    def _parse_bal(self) -> ast.Statement:
        condition = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after condition register")

        r_jmp_target = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after jump target register")

        r_link = self._parse_register()

        return ast.InstBAL(condition, r_jmp_target, r_link)

    def _parse_int(self) -> ast.Statement:
        r_service_id = self._parse_register()
        return ast.InstINT(r_service_id)

    def _parse_clr(self) -> ast.Statement:
        rd = self._parse_register()
        return ast.InstCLR(rd)

    def _parse_db(self) -> ast.Statement:
        data = self._consume(TokenType.STRING, "Expected a string after DB")
        return ast.InstDB(cast(str, data.literal).encode("utf-8"))

    def _parse_hlt(self) -> ast.Statement:
        return ast.InstHLT()

    def _parse_jmp(self) -> ast.Statement:
        r_jmp_target = self._parse_register()
        return ast.InstJMP(r_jmp_target)

    def _parse_ll(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")

        rt = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after temporary register")

        label = self._consume(TokenType.IDENTIFIER, "Expected a label name")

        return ast.InstLL(rd, rt, label.lexeme)

    def _parse_mov(self) -> ast.Statement:
        rd = self._parse_register()
        self._consume(TokenType.COMMA, "Expected ',' after destination register")
        rs = self._parse_register()

        return ast.InstMOV(rd, rs)

    def _parse_label_definition(self) -> ast.Statement:
        label = self._consume(TokenType.IDENTIFIER, "Expected a label name")
        self._consume(TokenType.COLON, "Expected ':' after label name")
        return ast.LabelDefinition(label.lexeme)

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
            self._line_no = self._peek().line_no
            return True
        return False

    def _consume(self, token_type: TokenType, error_msg: str) -> Token:
        if self._peek_type() == token_type:
            token = self._peek()
            self._current_pos += 1
            self._line_no = self._peek().line_no
            return token
        raise RuntimeError(error_msg)

    def _is_at_end(self) -> bool:
        return self._peek_type() == TokenType.EOF
