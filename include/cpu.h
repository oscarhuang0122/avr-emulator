// cpu.h
// Simulate CPU Registers  

#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define NUM_REGISTER 32  // Assuming 32 general-purpose registers

typedef struct CPU
{
    uint8_t registers[NUM_REGISTER];  // General-purpose registers
    uint16_t pc;                      // Program counter
    uint16_t sp;                       // Stack pointer
    uint8_t sreg;                     // Status register
    uint8_t interrupt_pending;        // Interrupt pending flag
    uint16_t interrupt_vector;        // Interrupt vector address
}CPU;

void cpu_reset(CPU *cpu);
void cpu_print_state(CPU *cpu);

#endif
