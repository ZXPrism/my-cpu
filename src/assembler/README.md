# Design doc

## Pseudoinstructions (260801)
I realized a caveat when implementing the assembler.

The ISA is small for the sake of simplicity and we need pseudoinstructions to support some high level yet common operations.

Pseudoinstructions are useful, but do not give assembler too much freedom,
namely, to use registers "smartly".

That's actually stupid because we can never assume if a reg is used.

However, high level langauge compilers do not suffer from this, since they manage the lifecycle of all registers.

Rule for this part: pseudoinstructions should NEVER alter unrelated registers.

Currently supported pseudoinstructions:
- HLT: halt
- MOV rd, rs: R[rd] <- R[rs]
- CLR r: R[r] <- 0
- JMP r: JAL x0, R[r]
- DB: define bytes
  - DB "hello, world!"
  - DB 23, 17, 255, 1, 1, 4, 5, 1, 4
- LL rd, rt, label: load label (u16), R[rd] <- label via temporary register R[rt]


## PseudoInstruction Expansion (260809)
Instruction validation should happen before pseudoinstruction expansion, or the error logs will be confusing.

## TODO
- [ ] `260809` Support hex literals
