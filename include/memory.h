// memory.h
// Implement Memory Map

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

//typedef struct CPU CPU;

#include "avr_defs.h"
#include "cpu.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "eeprom.h"
#include "wdt.h"

#define RAM_SIZE 2048
#define ROM_SIZE 32768

/*
// GPIO
#define PORTB_ADDR  0x25
#define DDRB_ADDR   0x24
#define PINB_ADDR   0x23

// CPU / Interrupt
#define SREG_ADDR   0x5F  // AVR's status register address
#define EIMSK_ADDR  0x3D  // External Interrupt Mask Register
#define EIFR_ADDR   0x3C  // External Interrupt Flag Register

// Timer0
#define TIFR0_ADDR  0x35
#define TCCR0A_ADDR 0x44
#define TCNT0_ADDR  0x46
#define OCR0A_ADDR  0x47
#define TIMSK0_ADDR 0x6E

// ADC
#define ADMUX_ADDR  0x7C
#define ADCSRA_ADDR 0x7A
#define ADCL_ADDR   0x78
#define ADCH_ADDR   0x79

// SPI
#define SPCR_ADDR   0x4C
#define SPSR_ADDR   0x4D
#define SPDR_ADDR   0x4E

// I2C
#define TWBR_ADDR   0xB8
#define TWSR_ADDR   0xB9
#define TWAR_ADDR   0xBA
#define TWDR_ADDR   0xBB
#define TWCR_ADDR   0xBC

*/

typedef struct Memory
{
    uint8_t rom[ROM_SIZE];
    uint8_t ram[RAM_SIZE];

    CPU *cpu;

    uint8_t int_mask;
    uint8_t int_flags;
    
    // Peripherals
    GPIO gpio;
    UART uart;
    Timer timer;
    ADC adc;
    SPI spi;
    I2C i2c;
    EEPROM eeprom;
    WDT wdt;
    
} Memory;

void memory_init(Memory *mem);
uint8_t mem_read(Memory *mem, uint16_t addr);
void mem_write(Memory *mem, uint16_t addr, uint8_t value);


#endif