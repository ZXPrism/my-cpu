from dataclasses import dataclass
from enum import IntEnum


class TokenType(IntEnum):
    # Single-character tokens
    COLON = -1  # :
    COMMA = 0  # ,
    SEMICOLON = 1  # ;

    # Literals
    IDENTIFIER = 2
    STRING = 3
    NUMBER = 4

    # Instruction Mnemonics
    ADD = 5
    SUB = 6
    SLL = 7
    SRL = 8
    SRA = 9
    SLT = 10
    SLTU = 11
    XOR = 12
    OR = 13
    AND = 14
    STR = 15
    LDR = 16
    LI = 17
    JAL = 18
    BAL = 19
    INT = 20

    # Pseudo-Instruction Mnemonics
    CLR = 21
    DB = 22
    HLT = 23
    JMP = 24
    LL = 25
    MOV = 26

    # Special
    EOF = 27


@dataclass
class Token:
    type: TokenType
    lexeme: str
    literal: object | None
    line_no: int

    def to_string(self) -> str:
        return f"{self.type.name} {self.lexeme} {self.literal}"
