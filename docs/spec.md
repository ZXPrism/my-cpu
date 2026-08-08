# CPU Project Specification (Rev. C - 20260808)

## 1. Project Intent

This project will design and build a custom CPU for education and personal
curiosity. It is a personal, learn-by-doing project: decisions may be deferred
until they are needed and then refined through implementation.

All design decisions are revisable during development. Changes may extend to
the ISA and hardware design when experience reveals a better direction. This
flexibility is intentional because the project is casual, but the design,
implementation, and documentation should still maintain professional rigor.

Tool choices may be deliberately limited according to personal preference.
Compiler construction itself is not a learning objective of this project, so
existing compiler infrastructure should be reused instead of reimplemented
where practical.

## 2. Project Organization

The project will be divided into multiple stages. Each stage will have explicit
completion criteria.

Stages are identified in Greek alphabetical order. The current partitioning is
rough and may be revised during development.

Current stage: **Beta - Virtual Machine (in progress)**

### 2.1 Alpha: ISA Design

The alpha stage will design the instruction set architecture.
This stage is complete. Its ISA specification may still be revised during
later development.

Deliverable:

- the ISA specification.

The ISA will follow the reduced instruction set computer (RISC) design
philosophy. It is inspired by RV32I but is a distinct, incompatible ISA.

### 2.2 Beta: Virtual Machine

The beta stage will implement a virtual machine that reliably conforms to the
ISA. This stage is in progress.

Deliverables:

- the virtual machine; and
- demonstration programs running on the virtual machine.

Potential demonstration programs include computing the Fibonacci sequence and
printing the program itself. The final demonstration set is to be determined.

### 2.3 Gamma: Assembler

The gamma stage will implement the assembler. Pseudoinstructions will be
introduced and designed during this stage.

Deliverable:

- the assembler.

### 2.4 Delta: Rust Backend

The delta stage will add a custom backend for Rust targeting the CPU.

Deliverable:

- a Rust backend capable of generating code for the CPU.

The STM32F103C8T6-based hardware prototype originally planned for gamma is
postponed to a later stage. Its new stage identifier and deliverables are to be
determined. Other stages after delta and their deliverables are also to be
determined. The detailed task schedule and completion criteria have not yet
been defined.

## 3. Alpha ISA Specification

### 3.1 Scope

The CPU is a 16-bit RISC machine with a fixed 16-bit instruction width. The
ISA contains exactly 16 instruction types.

The instruction mnemonics and operand lists in this section are semantic
notation. The final assembly-language syntax is deferred.

### 3.2 Fundamental Units

- The architectural data width is 16 bits.
- Every instruction is exactly 16 bits and occupies two consecutive byte
  addresses.
- An architectural memory address identifies one 8-bit byte.
- Memory addresses and the program counter are 16 bits wide.
- Register identifiers and opcodes are 4 bits wide.

The architectural address space therefore contains 65,536 bytes
(`0x0000` through `0xFFFF`), equivalent to 64 KiB of storage.

All 16-bit instruction encodings and multi-byte data representations are
little-endian:

```text
memory[address]     = value[7:0]
memory[address + 1] = value[15:8]
```

The starting address of every 16-bit instruction fetch shall be even. Valid
instruction addresses therefore range from `0x0000` through `0xFFFE`. An odd
instruction address is architecturally invalid.

`LDR` and `STR` each access one byte and impose no alignment restriction. Every
address from `0x0000` through `0xFFFF` is valid for a data access.

### 3.3 Architectural State

#### 3.3.1 General-Purpose Registers

The register file contains 16 registers named `x0` through `x15`. Each register
is 16 bits wide and is selected by a 4-bit register identifier.

- `x0` always reads as `0x0000`.
- Writes to `x0` are discarded.
- `x1` through `x15` are general-purpose mutable registers.

#### 3.3.2 Program Counter

The program counter (`PC`) is a separate 16-bit register containing the byte
address of the current instruction.

#### 3.3.3 Memory

Instruction and data accesses share one unified, byte-addressed memory.
`LDR` and `STR` may therefore read and modify bytes that contain instructions.

### 3.4 Instruction Encoding

Every instruction has the following layout:

```text
 15            12 11             8 7              4 3              0
+----------------+----------------+----------------+----------------+
| opcode         | field A        | field B        | field C        |
+----------------+----------------+----------------+----------------+
```

The opcode occupies the most-significant four bits. It alone determines the
instruction type; the ISA has no secondary function fields.

| Opcode | Instruction | Field A          | Field B           | Field C           |
| :----: | ----------- | ---------------- | ----------------- | ----------------- |
| `0x0`  | `ADD`       | destination      | source 1          | source 2          |
| `0x1`  | `SUB`       | destination      | source 1          | source 2          |
| `0x2`  | `SLL`       | destination      | value             | shift count       |
| `0x3`  | `SRL`       | destination      | value             | shift count       |
| `0x4`  | `SRA`       | destination      | value             | shift count       |
| `0x5`  | `SLT`       | destination      | source 1          | source 2          |
| `0x6`  | `SLTU`      | destination      | source 1          | source 2          |
| `0x7`  | `XOR`       | destination      | source 1          | source 2          |
| `0x8`  | `OR`        | destination      | source 1          | source 2          |
| `0x9`  | `AND`       | destination      | source 1          | source 2          |
| `0xA`  | `STR`       | value            | address           | unused            |
| `0xB`  | `LDR`       | destination      | address           | unused            |
| `0xC`  | `LI`        | destination      | immediate `[7:4]` | immediate `[3:0]` |
| `0xD`  | `JAL`       | link destination | target            | unused            |
| `0xE`  | `BAL`       | condition        | target            | link destination  |
| `0xF`  | `INT`       | service          | unused            | unused            |

Used operand fields are packed toward the most-significant end of the operand
area. Unused fields are always the least-significant fields. An assembler
shall encode unused fields as zero; the CPU shall ignore their values.

All 16 opcode values are assigned. Consequently, every 16-bit instruction
encoding identifies a valid instruction and the ISA has no
illegal-instruction encoding.

### 3.5 General Execution Semantics

Instructions execute architecturally one at a time and appear atomic,
irrespective of the number of hardware cycles used by an implementation.

For an instruction at address `p`:

1. The CPU fetches the little-endian 16-bit instruction beginning at the even
   byte address `p`.
2. All source register values are read before any destination register is
   written.
3. The instruction performs its register, memory, and control-flow effects.

Unless an instruction specifies another value, the next program counter is:

```text
PC <- (p + 2) modulo 65536
```

The increment is two bytes because every instruction is 16 bits wide.
Sequential execution after the instruction at `0xFFFE` continues at
`0x0000`.

All ordinary arithmetic results are reduced modulo 65,536. Signed operations
interpret 16-bit values using two's-complement representation. The ISA has no
arithmetic flags and does not raise arithmetic-overflow exceptions.

### 3.6 Instruction Semantics

In the definitions below, `R[n]` is the value read from register `xn`,
`R[n][15:8]` and `R[n][7:0]` denote its high and low bytes respectively, and
`s16(v)` is the signed two's-complement interpretation of the 16-bit value
`v`.

#### 3.6.1 Arithmetic, Shift, Comparison, and Logic

| Instruction    | Effect                                                                                          |
| -------------- | ----------------------------------------------------------------------------------------------- |
| `ADD A, B, C`  | `R[A] <- (R[B] + R[C]) modulo 65536`                                                            |
| `SUB A, B, C`  | `R[A] <- (R[B] - R[C]) modulo 65536`                                                            |
| `SLL A, B, C`  | `R[A] <- (R[B] << (R[C] AND 0x000F)) modulo 65536`                                              |
| `SRL A, B, C`  | Logically shift `R[B]` right by `R[C] AND 0x000F`; write the result to `R[A]`.                  |
| `SRA A, B, C`  | Arithmetically shift `s16(R[B])` right by `R[C] AND 0x000F`; write the 16-bit result to `R[A]`. |
| `SLT A, B, C`  | Write `0x0001` to `R[A]` if `s16(R[B]) < s16(R[C])`; otherwise write `0x0000`.                  |
| `SLTU A, B, C` | Write `0x0001` to `R[A]` if unsigned `R[B] < R[C]`; otherwise write `0x0000`.                   |
| `XOR A, B, C`  | `R[A] <- R[B] XOR R[C]`                                                                         |
| `OR A, B, C`   | `R[A] <- R[B] OR R[C]`                                                                          |
| `AND A, B, C`  | `R[A] <- R[B] AND R[C]`                                                                         |

Only the least-significant four bits of a shift-count register affect a shift.
The effective shift count is therefore in the range 0 through 15.

#### 3.6.2 Memory

| Instruction | Effect                                             |
| ----------- | -------------------------------------------------- |
| `STR A, B`  | `memory[R[B]] <- R[A][7:0]`                        |
| `LDR A, B`  | `R[A] <- R[A][15:8] || memory[R[B]]`               |

Each memory instruction transfers exactly one byte at the address in its
address register. There is no immediate address offset. `LDR` replaces only
the low byte of its destination register and preserves that register's high
byte. `STR` reads only the low byte of its value register. Neither instruction
requires an aligned address.

#### 3.6.3 Immediate Load

`LI A, imm8` replaces the low byte of `R[A]` with the 8-bit immediate and
preserves its high byte:

```text
R[A] <- R[A][15:8] || imm8
```

`LI` is the only real instruction containing an immediate operand.

#### 3.6.4 Control Flow

`JAL A, B` reads the target before writing the link register:

```text
target <- R[B]
R[A]   <- (p + 2) modulo 65536
PC     <- target
```

Selecting `x0` as field A discards the link and produces a plain
register-indirect jump. The target address shall be even.

`BAL A, B, C` branches only when the condition register contains exactly
`0x0001`. The condition and target are read before the link is written:

```text
if R[A] == 0x0001:
    target <- R[B]
    R[C]   <- (p + 2) modulo 65536
    PC     <- target
else:
    PC     <- (p + 2) modulo 65536
```

The link register is unchanged when the branch is not taken. Selecting `x0`
as field C discards the link when the branch is taken. A taken branch target
shall be even.

#### 3.6.5 Environment Service

`INT A` synchronously requests an execution-environment service identified by
`R[A]`. The continuation address is `(p + 2) modulo 65536`.

The requested service is responsible for preserving or restoring the required
machine state and for returning to the correct continuation address. A service
may intentionally not return, for example when terminating execution. Service
identifiers, arguments, results, and permitted state changes belong to a
separate execution-environment ABI.

### 3.7 Reset and Exceptional Behavior

On CPU reset:

- `PC` is set to `0x0000`;
- `x0` remains fixed at `0x0000`;
- `x1` through `x15` have unspecified values; and
- memory is not cleared or otherwise modified.

Memory contents are established externally before execution.

Fetching an instruction at an odd address is architecturally invalid. Hardware
behavior for an invalid instruction fetch is not specified, and conforming
software shall not rely on it. The virtual machine shall diagnose such a fetch
rather than silently aligning it. `LDR` and `STR` are valid at both even and
odd addresses.

The ISA currently defines no external hardware interrupts or halt instruction.
An execution-environment service may provide termination.

### 3.8 Pseudoinstructions

Pseudoinstructions are assembler conveniences and do not add opcodes to the
ISA. Multiple real instructions may be used to construct a full 16-bit
constant or perform multi-byte memory operations. Their exact definitions are
deferred to the assembly-language specification.

## 4. Rust Toolchain

Rust is the high-level programming language for the CPU. A new programming
language and compiler frontend will not be developed.

- The existing Rust frontend will run on the host computer.
- A custom LLVM target/backend will generate code for the custom CPU.
- Supporting Rust target and runtime components will be provided as required.
- The implementation should make the greatest practical use of LLVM to avoid
  reimplementing established compiler functionality.

During early development, software for the CPU will be written in assembly
language. The assembly language will be specified after the instruction set
architecture (ISA) has been designed.

## 5. Software Behavioral Simulation

The early development stage will not involve hardware. A software virtual
machine will behaviorally simulate the CPU.

The virtual machine will be used to:

- test assembly programs and Rust programs compiled for the CPU; and
- regress later hardware implementations.

The virtual machine will be implemented in C++ and developed in multiple
stages. Its first stage will operate on semantic instruction records rather
than packed 16-bit instruction encodings. A record will represent an opcode
and its operand fields directly, allowing instruction behavior to be developed
before bit-level decoding.

The virtual machine will be exposed to Godot through GDExtension. Godot will
provide an interactive environment for writing assembly programs and
inspecting machine state.

At reset, the virtual machine will initialize `x1` through `x15` with random
values to discourage software from depending on unspecified register state.
It may maintain additional metadata and explicitly diagnose invalid or
undesirable operations. Such diagnostics are testing features and are not
architectural exceptions.

## 6. Hardware Development

### 6.1 STM32 Behavioral Prototype

The first hardware-oriented development stage will use several
STM32F103C8T6 development boards. The boards will serve as components of the
CPU hardware and provide behavioral simulation.

This prototype will be closer to the eventual hardware than the software
virtual machine, but it will remain an intermediate implementation and will
not be the final delivery.

### 6.2 Final Implementation

The final CPU will be physically constructed using:

- 74HC-series logic chips;
- memory chips;
- a custom power circuit; and
- other required supporting components.

## 7. Deferred Design Details

The following details are intentionally deferred:

- the assembly-language definition;
- pseudoinstruction expansions, including full-width constant construction and
  multi-byte loads and stores;
- the calling convention and other software ABI details;
- execution-environment service identifiers and conventions;
- the supported Rust subset, target data layout, and runtime support;
- virtual-machine stages after the initial semantic-instruction stage;
- the detailed project schedule and stage completion criteria;
- the final beta-stage demonstration programs;
- detailed gamma- and delta-stage completion criteria;
- the new stage identifier and deliverables for the STM32 prototype;
- other stages after delta and their deliverables; and
- CPU cycle-control and related hardware design decisions.
