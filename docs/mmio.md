# Memory-Mapped I/O

The key idea behind memory-mapped I/O is:

> A store instruction does not inherently mean "write to RAM." It means "issue
> a write transaction to an address."

The hardware address map determines what receives that transaction.

## From a store instruction to a bus transaction

Suppose a program executes:

```asm
STR x1, x2
```

and `x2` contains `0xFF00`, an address assigned to a UART output register. The CPU
presents signals conceptually like these to its memory interface:

```text
address      = 0xFF00
write data   = low byte of x1
write enable = 1
request      = 1
```

The CPU does not need to know whether the address refers to RAM or a peripheral.
It only issues the transaction.

## Address decoding and routing

The memory interface connects to an address decoder or bus interconnect:

```text
                    ┌─ RAM
CPU memory request ─┼─ UART
                    ├─ timer
                    └─ display controller
```

The system's address map might be:

```text
0x0000-0xEFFF → RAM
0xF000-0xF0FF → UART
0xF100-0xF1FF → timer
0xF200-0xF2FF → display controller
```

The decoder examines each request and selects the appropriate destination. A
write to `0xFF00` activates the UART rather than RAM:

```text
STR
 ↓
(address, data, write-enable)
 ↓
address decoder
 ├── RAM range       → store the data
 ├── UART range      → transmit a byte
 ├── display range   → change a pixel or device register
 ├── timer range     → configure the timer
 └── unmapped range  → report an error
```

Even RAM is simply one device attached to the address space.

Conceptually, the hardware selection logic could look like:

```verilog
always_ff @(posedge clock) begin
    if (memory_write && address == 16'hFF00) begin
        uart_tx_data <= write_data;
        uart_tx_start <= 1;
    end
end
```

The UART sees its selection signal, captures the byte, and acknowledges the
transaction.

## Bus handshaking

A realistic memory bus commonly has signals conceptually like:

```text
CPU/device request:
    valid
    address
    write_enable
    write_data

Device response:
    ready
    read_data
    error
```

The CPU asserts `valid`. The selected device asserts `ready` when it has accepted
the write or produced the requested read value. The CPU may stall until that
happens. Bus protocols such as AXI, AHB, and Wishbone formalize variations of
this interaction.

## Device registers can cause side effects

A memory-mapped register does not necessarily behave like storage. A write may
trigger an action without preserving the written value:

```text
write UART_DATA   → transmit a character
write TIMER_START → start a countdown
write GPU_COMMAND → begin a rendering operation
```

A UART write may continue through:

```text
CPU store
  ↓
UART transmit register
  ↓
UART transmit FIFO
  ↓
bit-shifting circuit
  ↓
physical TX pin
  ↓
serial receiver
  ↓
terminal program
```

If the device cannot immediately accept more data, it may delay the transaction,
expose a not-ready status bit, or discard data, depending on its design.

## Reads can expose live device state

A UART could expose several addresses:

```text
0xFF00 → transmit/receive data
0xFF01 → status
0xFF02 → configuration
```

Reading `0xFF01` does not retrieve an ordinary RAM byte. The UART generates status
bits representing its current state:

```text
bit 0: transmit buffer ready
bit 1: received byte available
```

Software can therefore poll the device:

```text
wait until UART_STATUS says ready
write character to UART_DATA
```

## Why a memory watcher is unnecessary

One possible simulator design is:

```text
write RAM → watcher notices change → watcher activates device
```

That can work, but it models the interaction indirectly. MMIO models the hardware
more naturally:

```text
write address → interconnect routes request directly to device
```

The device receives the transaction when it happens; it does not periodically
search RAM for changes.

## Modeling MMIO in the VM

The VM can model address decoding directly in its implementation of `STR`:

```cpp
if (address == UART_DATA_ADDRESS) {
    output_character(value);
} else {
    mem[address] = value;
}
```

This conditional represents, at a high level, the physical address decoder, bus
routing, and UART device.

## The architectural insight

A memory address is not inherently RAM. It identifies a destination in an
addressed transaction. The CPU can see one unified address space containing:

- RAM
- ROM
- Device registers
- Configuration registers
- Unmapped regions

Address routing therefore turns ordinary load and store instructions into a
general interface to the entire machine.
