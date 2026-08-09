from enum import IntEnum

import asm.ast as ast

from rich import print


class OpCode(IntEnum):
    ADD = 0
    SUB = 1
    SLL = 2
    SRL = 3
    SRA = 4
    SLT = 5
    SLTU = 6
    XOR = 7
    OR = 8
    AND = 9
    STR = 10
    LDR = 11
    LI = 12
    JAL = 13
    BAL = 14
    INT = 15


class Codegen:
    def __init__(self, program: ast.Program):
        self._program = program
        self.map_symbol_to_addr: dict[str, int] = {}

    def _resolve_symbols(self):
        current_addr = 0

        for stmt in self._program.statements:
            match stmt:
                case ast.LabelDefinition(name):
                    if name in self.map_symbol_to_addr:
                        raise RuntimeError(
                            f"Found duplicate definition of label {name}"
                        )
                    self.map_symbol_to_addr[name] = current_addr
                case _:
                    current_addr += self._get_encoded_size_bytes(stmt)

    def _get_encoded_size_bytes(self, stmt: ast.Statement) -> int:
        match stmt:
            case ast.InstDB():
                return len(stmt.data)
            case ast.InstLL():
                # === LL expansion rule ===
                # LL rd, rt, label <=>
                # LI rd, label[15:12], label[11:8]
                # LI rt, 0, 8
                # SLL rd, rd, rt
                # LI rt, label[7:4], label[3:0]
                # ADD rd, rd, rt
                return 12
            case ast.LabelDefinition():
                raise RuntimeError("This branch should be unreachable")
            case _:
                return 2

    def _encode_bytecode(self) -> bytearray:
        bytecode = bytearray()

        for stmt in self._program.statements:
            match stmt:
                case ast.InstADD(rd, rs1, rs2):
                    inst = (OpCode.ADD << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                # TODO: populate rest cases (ignore LabelDefinition)

        return bytecode

    def emit(self, bytecode_file_path: str):
        self._resolve_symbols()
        bytecode = self._encode_bytecode()
        with open(bytecode_file_path, "wb") as fp:
            fp.write(bytecode)
