# Assembler Learning Notes

Building the assembler exposed several important boundaries, and each boundary
revealed a different class of trap.

## 1. An assembler AST is flat

Assembly naturally becomes an ordered list rather than a deeply nested tree:

```text
Program
├── Instruction
├── LabelDefinition
├── Instruction
└── ...
```

The small amount of nesting that exists is inside operands, not statements.

`Statement` does not need to be a base class. It can be a type union describing
what may appear in `Program.statements`:

```python
Statement = InstADD | InstSUB | ... | LabelDefinition
```

The initial union listed only `InstADD`, `InstSUB`, and `LabelDefinition`, even
though the AST contained many more node types. The union must stay exhaustive if
it is to describe the AST accurately.

## 2. Recursive descent need not literally recurse

For this grammar, recursive descent means descending through grammar functions:

```text
parse
  → parse_statement
    → parse_add
      → parse_register
```

Because assembly is flat and has no nested expressions, the implementation may
perform no actual recursive calls. It is still the recursive-descent style.

## 3. Trying and requiring are different parser operations

```python
_match(TokenType.ADD)
```

means:

> Try this alternative; consume it only if it matches.

Whereas:

```python
_consume(TokenType.COMMA, ...)
```

means:

> The grammar requires this token here; failure means malformed source.

This distinction separates grammar selection from grammar enforcement.

Related parser details included:

- Stop when the next token is `EOF`, not only after stepping beyond the list.
- Return the completed `Program`.
- Recognize `IDENTIFIER COLON` as a label definition.
- Convert register-shaped identifiers such as `x4` into `Register(4)`.

## 4. Python class-pattern syntax has a subtle trap

This does not match `LabelDefinition` instances:

```python
case ast.LabelDefinition:
```

Python treats `ast.LabelDefinition` as a value and compares the statement to the
class object itself. A class-instance pattern requires parentheses:

```python
case ast.LabelDefinition():
```

It can also destructure the dataclass:

```python
case ast.LabelDefinition(name):
```

The same rule applies to `InstDB`, `InstLL`, and every other AST class used in a
`match` statement.

## 5. Label resolution depends on encoded size

A label's address cannot be calculated by simply counting AST statements:

- Labels occupy zero bytes.
- Intrinsic instructions occupy two bytes.
- `DB` occupies however many bytes it contains.
- Pseudoinstructions occupy the size of their expansions.
- `LL` expands into multiple instructions.

This leads naturally to a two-pass model:

```text
Pass 1: calculate layout and collect label addresses
Pass 2: resolve labels and emit bytecode
```

Label addresses are byte addresses, matching the VM program counter, which
advances by two bytes per instruction.

## 6. Bit layout and byte order are separate contracts

The instruction word has this structure:

```text
15       12 11        8 7         4 3         0
+----------+-----------+-----------+-----------+
|  opcode  |  field A  |  field B  |  field C  |
+----------+-----------+-----------+-----------+
```

The VM loads that word from memory in little-endian order:

```text
low byte first, high byte second
```

The `ADD` encoder got both contracts right. A correct instruction word can still
execute incorrectly if its bytes are emitted in the wrong order.

## 7. Pseudoinstructions must respect actual ISA semantics

A pseudoinstruction is not merely a textual shortcut. Its expansion must behave
correctly for every permitted register state.

The project rule is important:

> A pseudoinstruction must never unexpectedly alter an unrelated register.

That is why `LL` explicitly receives a temporary register.

There was also a subtle reason to clear that temporary register: `LI` modifies
only its low byte. Loading a low-byte value into a randomly initialized temporary
register would leave random high bits and corrupt the final address.

## 8. `LI` did not mean what the Fibonacci program assumed

The VM implements `LI` as:

```text
R[rd][7:0] = immediate
R[rd][15:8] remains unchanged
```

It does not zero-extend the immediate into the complete register.

Because the VM deliberately randomizes registers, this instruction:

```asm
LI x5, 1
```

might produce `0xAB01`, not `0x0001`. That affected:

- The Fibonacci state
- The counter increment
- The loop limit
- Memory addresses

The lesson is that assembly programs begin with no friendly assumptions. If an
instruction initializes only part of a register, the rest truly remains whatever
it previously contained.

## 9. "First ten Fibonacci numbers" needed a precise definition

With `a = 0`, `b = 1`, and storing `c = a + b`, the initial program generated:

```text
1, 2, 3, 5, 8, 13, 21, 34, 55, 89
```

It skipped the conventional second `1`. This was an algorithm-level issue rather
than an assembler issue, but running real assembled code exposed it.

There was also an initialized register, `x10`, that was unused.

## 10. The VM has unified code and data memory

The VM first randomizes memory, but loading the bytecode replaces the beginning
of that memory:

```text
memory[0:N] = program bytecode
```

The initial Fibonacci program then stored results at addresses `0-9`:

```text
memory[0:10] = Fibonacci output
```

The program therefore overwrote its own machine code. It happened to work
because those overwritten bytes belonged to initialization instructions that had
already executed. If the output grew far enough to reach the loop body, the
program could modify instructions it would execute again.

This was the project's first accidental self-modifying program.

The clean solution is to give data its own labeled region after the executable
instructions, or deliberately establish a separate data address range.

## Recap

These mistakes appeared at abstraction boundaries:

```text
tokens ↔ grammar
grammar ↔ AST
AST ↔ layout
layout ↔ encoding
encoding ↔ VM
program ↔ memory
```

Encountering their failure modes directly makes these boundaries much more
concrete than merely reading about them.
