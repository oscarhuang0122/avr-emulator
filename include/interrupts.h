// interrupts.h

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "cpu.h"
#include "memory.h"

// Interrupt vector addresses
#define RESET_VECTOR 0x0000
#define INTERRUPT_VECTOR 0x0010
#define ADC_INT_VECTOR 0x1A // Example address for ADC interrupt

// Interrupt flags
#define INT0 0 // External Interrupt 0
#define INT1 1 // External Interrupt 1

void interrupt_init(Memory *mem);
void interrupt_trigger(Memory *mem, uint8_t interrupt);
//void handle_interrupt(CPU *cpu, Memory *mem);
void raise_interrupt(Memory *mem, uint16_t vector_addr);

#endif