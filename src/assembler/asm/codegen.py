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
                case ast.InstSUB(rd, rs1, rs2):
                    inst = (OpCode.SUB << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSLL(rd, rs, r_shift):
                    inst = (OpCode.SLL << 12) | (rd.idx << 8) | (rs.idx << 4) | r_shift.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSRL(rd, rs, r_shift):
                    inst = (OpCode.SRL << 12) | (rd.idx << 8) | (rs.idx << 4) | r_shift.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSRA(rd, rs, r_shift):
                    inst = (OpCode.SRA << 12) | (rd.idx << 8) | (rs.idx << 4) | r_shift.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSLT(rd, rs1, rs2):
                    inst = (OpCode.SLT << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSLTU(rd, rs1, rs2):
                    inst = (OpCode.SLTU << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstXOR(rd, rs1, rs2):
                    inst = (OpCode.XOR << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstOR(rd, rs1, rs2):
                    inst = (OpCode.OR << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstAND(rd, rs1, rs2):
                    inst = (OpCode.AND << 12) | (rd.idx << 8) | (rs1.idx << 4) | rs2.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstSTR(rs, rd_addr):
                    inst = (OpCode.STR << 12) | (rs.idx << 8) | (rd_addr.idx << 4)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstLDR(rd, rs_addr):
                    inst = (OpCode.LDR << 12) | (rd.idx << 8) | (rs_addr.idx << 4)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstLI(rd, imm):
                    inst = (OpCode.LI << 12) | (rd.idx << 8) | imm
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstJAL(r_link, r_jmp_target):
                    inst = (OpCode.JAL << 12) | (r_link.idx << 8) | (r_jmp_target.idx << 4)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstBAL(condition, r_jmp_target, r_link):
                    inst = (
                        (OpCode.BAL << 12)
                        | (condition.idx << 8)
                        | (r_jmp_target.idx << 4)
                        | r_link.idx
                    )
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstINT(r_service_id):
                    inst = (OpCode.INT << 12) | (r_service_id.idx << 8)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstCLR(rd):
                    inst = (OpCode.XOR << 12) | (rd.idx << 8) | (rd.idx << 4) | rd.idx
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstDB(data):
                    bytecode.extend(data)
                case ast.InstHLT():
                    inst = OpCode.INT << 12
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstJMP(r_jmp_target):
                    inst = (OpCode.JAL << 12) | (r_jmp_target.idx << 4)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.InstLL(rd, rt, label):
                    label_addr = self.map_symbol_to_addr[label]
                    instructions = [
                        (OpCode.LI << 12) | (rd.idx << 8) | ((label_addr >> 8) & 0xFF),
                        (OpCode.XOR << 12) | (rt.idx << 8) | (rt.idx << 4) | rt.idx,
                        (OpCode.LI << 12) | (rt.idx << 8) | 8,
                        (OpCode.SLL << 12) | (rd.idx << 8) | (rd.idx << 4) | rt.idx,
                        (OpCode.LI << 12) | (rt.idx << 8) | (label_addr & 0xFF),
                        (OpCode.ADD << 12) | (rd.idx << 8) | (rd.idx << 4) | rt.idx,
                    ]
                    for inst in instructions:
                        bytecode.append(inst & 0xFF)
                        bytecode.append(inst >> 8)
                case ast.InstMOV(rd, rs):
                    inst = (OpCode.ADD << 12) | (rd.idx << 8) | (rs.idx << 4)
                    bytecode.append(inst & 0xFF)
                    bytecode.append(inst >> 8)
                case ast.LabelDefinition():
                    pass

        return bytecode

    def emit(self, bytecode_file_path: str):
        self._resolve_symbols()
        bytecode = self._encode_bytecode()
        with open(bytecode_file_path, "wb") as fp:
            fp.write(bytecode)
