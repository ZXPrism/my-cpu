from dataclasses import dataclass
from typing import TypeAlias


@dataclass
class Register:
    idx: int


# ========================
#  Intrinsic Instructions
# ========================


@dataclass
class InstADD:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstSUB:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstSLL:
    rd: Register
    rs: Register
    r_shift: Register


@dataclass
class InstSRL:
    rd: Register
    rs: Register
    r_shift: Register


@dataclass
class InstSRA:
    rd: Register
    rs: Register
    r_shift: Register


@dataclass
class InstSLT:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstSLTU:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstXOR:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstOR:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstAND:
    rd: Register
    rs1: Register
    rs2: Register


@dataclass
class InstSTR:
    rs: Register
    rd_addr: Register


@dataclass
class InstLDR:
    rd: Register
    rs_addr: Register


@dataclass
class InstLI:
    rd: Register
    imm: int


@dataclass
class InstJAL:
    r_link: Register
    r_jmp_target: Register


@dataclass
class InstBAL:
    condition: Register
    r_jmp_target: Register
    r_link: Register


@dataclass
class InstINT:
    r_service_id: Register


# =====================
#  Pseudo-Instructions
# =====================


@dataclass
class InstCLR:
    rd: Register


@dataclass
class InstDB:
    data: bytes


@dataclass
class InstHLT:
    pass


@dataclass
class InstJMP:
    r_jmp_target: Register


@dataclass
class InstLL:
    rd: Register
    label: str


@dataclass
class InstMOV:
    rd: Register
    rs: Register


@dataclass
class LabelDefinition:
    name: str


Statement: TypeAlias = InstADD | InstSUB | LabelDefinition


@dataclass
class Program:
    statements: list[Statement]
