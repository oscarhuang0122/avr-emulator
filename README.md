# AVR Emulator

A C-based educational emulator for a subset of the AVR instruction set, with simulated memory-mapped peripherals and interrupt handling. The project runs on a host computer and includes a small AVR guest firmware example compiled for the ATmega328P.

> **Scope:** This is a partial, instruction-level emulator and peripheral simulation, **not** a cycle-accurate or fully compatible ATmega328P simulator.

## Features

- CPU model with 32 general-purpose registers, program counter (PC), stack pointer (SP), and status register (SREG)
- Instruction execution for a supported subset including `LDI`, `IN`, `OUT`, `ADD`, `SUB`, `SUBI`, `BRNE`, `RJMP`, `PUSH`, `POP`, `RCALL`, `RET`, `RETI`, `LDS`, and `STS`
- Stack operations, status flags, and simplified interrupt entry/return
- ROM and RAM with memory-mapped register access
- Simulated GPIO, UART, SPI, I2C, timer, ADC, EEPROM, and watchdog modules
- Basic Intel HEX loader for guest firmware
- Unit tests and a CPU/GPIO/interrupt integration test

## Architecture

See [architecture diagram](docs/architecture.md) for the execution flow and module relationships.

At a high level, the host program loads guest firmware into emulated ROM, executes supported AVR instructions against CPU and memory state, and advances simulated peripherals in the execution loop. Interrupt handling uses the emulated CPU state and stack.

## Requirements

For Ubuntu or WSL Ubuntu:

- `gcc` and `make`
- `avr-gcc` and `avr-objcopy` (typically installed via `gcc-avr` and `binutils-avr`)

Example setup:

```bash
sudo apt update
sudo apt install build-essential gcc-avr binutils-avr avr-libc
```

## Build and run

Run these commands from the repository root:

```bash
make clean
make all
make run
```

`make all` builds the host executable (`avr_emulator`) and compiles `src/program.c` into `program.hex`. `make run` executes the emulator. The program demonstrates peripheral behavior and then runs a bounded 1,000-instruction simulation. Output includes instruction traces, GPIO state, and simulated UART traffic.

If `program.hex` cannot be loaded, the host program falls back to a built-in test program.

## Tests

```bash
make test
```

The current suite contains **34 tests**:

| Suite | Tests |
| --- | ---: |
| GPIO | 6 |
| Stack | 5 |
| Instructions and integration | 23 |

The integration test checks a short program that configures GPIO, writes output data, services an interrupt through `RETI`, restores the stack, and resumes execution.

A successful local verification also included `make all` and `make run`, which exited with status `0` after 1,000 instructions.

## Repository structure

```text
include/             Public headers for CPU, memory, and peripherals
src/                 Emulator implementation, tests, and guest program
src/main.c           Host emulator entry point and demonstration loop
src/instructions.c   Instruction execution
src/memory.c         Memory access and peripheral register dispatch
src/hex_loader.c     Basic Intel HEX loader
src/program.c        AVR guest firmware example
src/test_*.c         Automated tests
Makefile             Build, test, run, and clean targets
docs/architecture.md Architecture diagram and notes
```

## Known limitations

- Only a subset of AVR instructions is implemented; arbitrary AVR binaries are not guaranteed to work.
- Peripherals are simplified software models rather than complete hardware-accurate implementations.
- Execution advances by instruction steps; it is not cycle-accurate.
- The HEX loader handles data and EOF records but does **not** currently validate checksums or support extended address records.
- The host demo is intentionally bounded to 1,000 instructions rather than running indefinitely.

## Development environment

Developed in C using GCC, Make, and the AVR GNU toolchain on Ubuntu/WSL.
