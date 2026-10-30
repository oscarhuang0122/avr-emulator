// interrupts.c

#include "interrupts.h"
#include "cpu.h"
#include "memory.h"
#include <stdio.h>

// Initialize interrupt-related flags and masks
void interrupt_init(Memory *mem)
{
    mem->int_flags = 0;  // Clear interrupt flags
    mem->int_mask = 0;  // Clear interrupt mask
}

// Trigger an interrupt
void interrupt_trigger(Memory *mem, uint8_t interrupt)
{
    // Set the corresponding interrupt flag
    if (interrupt < 8)  // We assume we can handle up to 8 interrupts
    {
        mem->int_flags |= (1 << interrupt);
    }
}

void raise_interrupt(Memory *mem, uint16_t vector_addr)
{
    if (mem && mem->cpu)
    {
        // Mark interrupt as pending
        mem->int_flags |= 0x01;
        // Set vector address for CPU to jump to when interrupt is serviced
        mem->cpu->interrupt_vector = vector_addr;
    }
}