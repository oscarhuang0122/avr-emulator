// avr_emulator.c
// main.c

#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "instructions.h"
#include "interrupts.h"
#include "timer.h"
#include "gpio.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "hex_loader.h"

#define MAX_INSTRUCTIONS 1000UL

void load_test_program(Memory *mem)
{
    // AVR Assembly:
    // LDI R16, 0xF0
    // OUT DDRB, R16   ; 0x24
    // LDI R17, 0xAA
    // OUT PORTB, R17  ; 0x25
    // IN R18, PINB    ; 0x23
    // RJMP -1         ; loop forever

    uint16_t program[] =
    {
        0xEF00,  // LDI R16, 0xF0
        0xB904,  // OUT DDRB, R16
        0xEA1A,  // LDI R17, 0xAA
        0xB915,  // OUT PORTB, R17
        0xB123,  // IN R18, PINB
        0xCFFF   // RJMP -1 loops forever; there is no NOP in this program.
    };

    memset(mem->rom, 0x00, ROM_SIZE);

    for (size_t i = 0; i < sizeof(program) / sizeof(program[0]); i++)
    {
        mem->rom[i * 2] = program[i] & 0xFF;
        mem->rom[i * 2 + 1] = (program[i] >> 8) & 0xFF;
    }

    // Interrupt handler at 0x0010
    mem->rom[0x10] = 0x18;
    mem->rom[0x11] = 0x95;
    
}

// SPI Test Simulation
void test_spi(Memory *mem)
{
    printf("\n--- [SPI TEST] ---\n");

    SPI master, slave;
    spi_init(&master);
    spi_init(&slave);

    // Configure master/slave
    master.spcr |= (1 << SPE) | (1 << MSTR);  // Enable + master
    slave.spcr |= (1 << SPE);                // Enable + slave
    
    // Connect them
    spi_connect(&master, &slave);

    // Master sends a byte
    spi_write(&master, SPDR_ADDR, 0x3C);

    // Step through "time"
    for (int i = 0; i < 10; i++)
    {
        spi_step(&master, mem);
        spi_step(&slave, mem);
    }

    printf("[SPI] Master received: 0x%02X\n", master.spdr);
    printf("[SPI] Slave  received: 0x%02X\n", slave.spdr);
}

// I2C Test Simulation
void test_i2C(Memory *mem)
{
    printf("\n--- [I2C TEST] ---\n");

    I2C slave_i2c;
    i2c_init(&slave_i2c);
    i2c_connect(&mem->i2c, &slave_i2c);

    i2c_write(&mem->i2c, TWCR_ADDR, (1 << TWEN) | (1 << TWSTA));  // Start
    i2c_write(&mem->i2c, TWDR_ADDR, 0x50);                        // Send byte
    i2c_write(&mem->i2c, TWCR_ADDR, (1 << TWEN));                 // Send
    for (int i = 0; i < 5; i++) i2c_step(&mem->i2c, mem);
    i2c_write(&mem->i2c, TWCR_ADDR, (1 << TWEN) | (1 << TWSTO));  // Stop

    printf("[I2C] Master transaction complete\n");
}

// UART Test Simulation
void test_uart(Memory *mem)
{
    printf("\n--- [UART TEST] ---\n");

    // Send a byte via UART (memory-mapped write)
    uart_send(&mem->uart, 0xAB);  // Example: send 0xAB
    printf("[UART] Sent: 0x%02X\n", 0xAB);

    // Simulate receiving a byte
    uart_simulate_receive(&mem->uart, 0xCD); // Simulate incoming data

    // Read received byte
    uint8_t received = uart_receive(&mem->uart);
    if (received != 0xFF)
        printf("[UART] Received: 0x%02X\n", received);
    else
        printf("[UART] No data received\n");
    
} 

void test_eeprom(Memory *mem)
{
    printf("\n--- [EEPROM TEST] ---\n");

    eeprom_write_byte(&mem->eeprom, 0x10, 0x42);

    for (int i = 0; i < 6; i++)
    {
        eeprom_step(&mem->eeprom);
    }

    uint8_t val = eeprom_read_byte(&mem->eeprom, 0x10);
    printf("[EEPROM] Final value at 0x10 = 0x%02X\n", val);
}

void test_wdt(Memory *mem)
{
    printf("\n--- [WDT TEST] ---\n");

    // Enable WDT with reset enabled
    wdt_write(&mem->wdt, (1 << WDE));

    // Simulate normal cycles
    for (int i = 0; i < 5; i++)
    {
        wdt_step(&mem->wdt, mem->cpu);
    }

    // Reset (kick) watchdog before timeout
    wdt_reset(&mem->wdt);

    // Let it time out to trigger reset
    for (int i = 0; i < 12; i++)
    {
        wdt_step(&mem->wdt, mem->cpu);
    }

    // Disable watchdog after standalone test
    wdt_write(&mem->wdt, 0);
}

int main()
{
    CPU cpu;
    Memory mem;

    // Initialize state for standalone peripheral tests
    cpu_reset(&cpu);
    memory_init(&mem);
    mem.cpu = &cpu;
    interrupt_init(&mem);

    // Run standalone peripheral demonstrations.
    test_spi(&mem);
    test_i2C(&mem);
    test_uart(&mem);
    test_eeprom(&mem);
    test_wdt(&mem);

    // Reset everything before running the AVR guest program.
    cpu_reset(&cpu);
    memory_init(&mem);
    mem.cpu = &cpu;
    interrupt_init(&mem);

    cpu.interrupt_vector = 0x0010;
    cpu.sreg |= (1 << 7);

    int result = load_hex("program.hex", &mem);
    
    if (result != 0)
    {
        printf("Failed to reload HEX; using built-in test program.\n");
        load_test_program(&mem);
    }
    else
    {
        printf("HEX program loaded.\n");
    }

    // Re-arm CPU interrupt state after watchdog test/reset
    cpu.sreg |= (1 << 7);
    cpu.interrupt_vector = 0x0010;
    mem.int_flags = 0;

    // --- CPU Emulation ---
    printf("\n--- [CPU EMULATION] ---\n");
    cpu_print_state(&cpu);

    printf("\n-- Executing Program --\n");

    for (unsigned long i = 0; i < MAX_INSTRUCTIONS; i++)
    {
        execute_instruction(&cpu, &mem);

        // Simulate GPIO, Timer, ADC and other peripherals.
        gpio_step(&mem.gpio);
        timer_step(&mem.timer, &mem);
        adc_step(&mem.adc, &mem);
        spi_step(&mem.spi, &mem);
        i2c_step(&mem.i2c, &mem);
        eeprom_step(&mem.eeprom);
        wdt_step(&mem.wdt, mem.cpu);

        if (i < 20 || i % 100 == 0)
        {
            printf("DDRB=0x%02X PORTB=0x%02X PINB=0x%02X\n", mem.gpio.ddrb, mem.gpio.portb, mem.gpio.pinb);
        }

        // Simulate incoming UART every 5 instructions.
        if (i % 5 == 0)
        {
            uint8_t simulated_byte = 0xA0 + (uint8_t)i;  // Example pattern: 0xA0, 0xA5, 0xAA
            uart_simulate_receive(&mem.uart, simulated_byte);
            printf("[UART SIM] Injected byte: 0x%02X\n", simulated_byte);
        }

        // Print received UART data
        uint8_t uart_data = uart_receive(&mem.uart);
        if (uart_data != 0xFF)
        {
            printf("[UART] Received : 0x%02X\n", uart_data);
        }
        
        // Trigger interrupt at instruction 3.
        if (i == 3)
        {
            printf("[TEST] Triggering interrupt\n");
            mem.int_flags |= (1 << 0);  // Set INTO flag
        }

        if (i < 20 || i % 100 == 0)
        {
            printf("\n[Instruction %lu]\n", i);
            cpu_print_state(&cpu);
            printf("\n");
        }
    }

    printf("Execution stopped after %lu instructions.\n", MAX_INSTRUCTIONS);

    return 0;
}