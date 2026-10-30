// cpu.c

#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"

#define RAM_SIZE 2048

void cpu_reset(CPU *cpu)
{
    if (cpu == NULL)
        return;

    memset(cpu->registers, 0, sizeof(cpu->registers));

    cpu->pc = 0;
    cpu->sp = RAM_SIZE - 1; // Set the stack pointer to the top of the 2KB RAM
    cpu->sreg= 0;  // Global Interrupt Enable
    cpu->interrupt_pending = 0;
    cpu->interrupt_vector = 0;
}

void cpu_print_state(CPU *cpu)
{
    printf("PC: 0x%04X  SP: 0x%04X  SREG: 0x%02X\n  INT: %s\n",
        cpu->pc, cpu->sp, cpu->sreg,
        cpu->interrupt_pending ? "PENDING" : "NONE");
    printf("Registers:\n");
    for (int i = 0; i < NUM_REGISTER; i++)
    {
        printf("R%02d: 0x%02X ", i, cpu->registers[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
}