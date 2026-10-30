# AVR Emulator architecture

```mermaid
flowchart TD
    Guest["Guest firmware: src/program.c"] --> Toolchain["avr-gcc / avr-objcopy"]
    Toolchain --> Hex["program.hex"]
    Hex --> Loader["Intel HEX loader"]
    Loader --> ROM["Emulated ROM"]
    Fallback["Built-in test program (fallback)"] --> ROM
    ROM --> Exec["Instruction executor"]
    Exec <--> CPU["CPU state: R0-R31, PC, SP, SREG"]
    Exec <--> Memory["Memory access layer"]
    Exec <--> Stack["Stack operations"]
    Stack <--> Memory
    Memory <--> RAM["Emulated RAM"]
    Memory <--> GPIO["GPIO"]
    Memory <--> UART["UART"]
    Memory <--> SPI["SPI"]
    Memory <--> I2C["I2C"]
    Memory <--> Timer["Timer"]
    Memory <--> ADC["ADC"]
    Memory <--> WDT["Watchdog"]
    Loop["Host execution loop: src/main.c"] --> Exec
    Loop --> Peripherals["Peripheral step functions"]
    Peripherals --> GPIO
    Peripherals --> SPI
    Peripherals --> I2C
    Peripherals --> Timer
    Peripherals --> ADC
    Peripherals --> WDT
    Peripherals --> EEPROM["EEPROM"]
    Timer --> Interrupts["Interrupt state / handling"]
    Interrupts --> CPU
    Interrupts --> Stack
```

## Execution flow

1. The host initializes CPU and memory/peripheral state.
2. The loader copies the compiled guest program from `program.hex` into emulated ROM. If loading fails, a built-in program is used.
3. The host repeatedly executes one instruction and advances the simulated peripherals.
4. Memory reads/writes dispatch selected register addresses to peripheral models; other valid addresses use emulated RAM.
5. Pending enabled interrupts transfer control to a configured vector and use the stack for the return address. `RETI` restores execution state.
6. The demonstration ends after 1,000 instruction steps.

This diagram shows the project's software organization, not exact AVR hardware timing or a complete ATmega328P bus model.
