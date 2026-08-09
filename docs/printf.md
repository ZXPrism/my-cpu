# How `printf` Works Across the Toolchain

Consider one ordinary call on Windows x64:

```c
#include <stdio.h>

int main(void) {
    int value = 42;
    printf("value = %d\n", value);
}
```

The compiler does not turn `printf` directly into "draw these characters." Many
layers cooperate:

```text
C source
  ↓ compiler
Object file with a call to an unresolved printf
  ↓ linker
Executable connected to the C runtime library
  ↓ OS loader
Running program
  ↓ printf implementation
Formatted character bytes
  ↓ Windows I/O
Console, file, or pipe
```

## 1. The header describes `printf`

`stdio.h` provides a declaration approximately like:

```c
int printf(const char *format, ...);
```

This tells the compiler:

- `printf` is a function.
- Its first argument is a string pointer.
- It accepts additional arguments.
- It returns an `int`.

The header generally does not contain the full implementation of `printf`.

## 2. The compiler handles the call

The compiler stores the format string somewhere in the program's read-only data:

```text
"value = %d\n\0"
```

It then generates machine instructions that pass the arguments according to the
Windows x64 ABI. Conceptually:

```text
RCX = address of "value = %d\n"
RDX = 42
CALL printf
```

The actual code also handles stack alignment and other ABI requirements.

At this point, the compiler may not know where `printf` will be located. The
generated object file therefore contains something like:

```text
There is a call here that must eventually target the symbol printf.
```

## 3. The linker resolves `printf`

The linker combines:

- Your compiled object file
- Startup/runtime code
- Required libraries
- Information about external dynamic libraries

It finds the C runtime's `printf` implementation. Depending on how the program is
linked, it either:

- Copies a static implementation into the executable, or
- Creates an import that will connect to a runtime DLL when the program starts

It also resolves addresses for the format string and other program data.

## 4. The OS loader starts the executable

When the program is run, Windows:

- Maps the executable into memory
- Maps required DLLs
- Resolves imported functions
- Initializes the C runtime
- Eventually transfers control to `main`

The call instruction now has a real route to the runtime's `printf`.

## 5. `printf` interprets the format

The C runtime's implementation receives:

```text
format pointer → "value = %d\n"
next argument  → 42
```

It scans the format string. Ordinary characters such as `"value = "` are copied
to an output buffer. When it reaches `%d`, it obtains the next argument according
to the ABI, interprets it as an integer, and converts:

```text
42 → '4', '2'
```

Finally, it processes the newline. The resulting character sequence is:

```text
value = 42\n
```

## 6. The runtime sends the characters to Windows

`printf` usually writes through the C runtime's `stdout` stream. The runtime may
buffer the output and eventually call a lower-level Windows I/O function.

Windows determines what `stdout` currently refers to:

- A terminal
- A redirected file
- A pipe to another program

If it is a terminal, the console system eventually renders the characters. If
output was redirected, the same `printf` call writes them into a file or pipe
instead.

## 7. The result returns through every layer

`printf` returns the number of characters written. That value is returned
according to the ABI—on Windows x64, normally in `EAX`.

Execution then resumes immediately after the original `CALL`.

## The equivalent chain on this CPU

The eventual toolchain would have the same conceptual structure:

```text
C compiler
  ↓ emits this ISA and follows its ABI
CALL printf
  ↓ resolved to the library implementation
printf parses the format and converts values
  ↓ calls putchar/write
putchar executes INT
  ↓
The VM receives INT and writes to the host terminal
```

Only the lowest step is inherently platform-specific:

```text
Program: "Please output this byte."
VM:      "I will ask Windows to display it."
```

Everything above that—including parsing `%d` and converting `42` into the bytes
for `'4'` and `'2'`—can be ordinary code running inside the VM.

`printf` is therefore not one magical instruction. It is a normal function
resting on a chain of contracts: C declarations, compiler-generated calls, ABI
argument placement, linker symbol resolution, runtime formatting code, and
finally an OS output operation.
