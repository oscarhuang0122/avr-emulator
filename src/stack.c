// stack.c

#include <stddef.h>
#include "stack.h"

bool stack_push8(CPU *cpu, Memory *mem, uint8_t value)
{
    if (cpu == NULL || mem == NULL)
        return false;

    if (cpu->sp ==0 || cpu->sp >= RAM_SIZE)
        return false;

    mem->ram[cpu->sp] = value;
    cpu->sp--;

    return true;
}

bool stack_pop8(CPU *cpu, Memory *mem, uint8_t *value)
{
    if (cpu == NULL || mem == NULL || value == NULL)
        return false;

    if (cpu->sp >= RAM_SIZE - 1)
        return false;

    cpu->sp++;
    *value = mem->ram[cpu->sp];

    return true;
}

bool stack_push16(CPU *cpu, Memory *mem, uint16_t value)
{
    if (cpu == NULL || mem == NULL)
        return false;
        
    if (cpu->sp < 2 || cpu->sp >= RAM_SIZE)
        return false;

    mem->ram[cpu->sp] = (uint8_t)((value >> 8) & 0xFF);
    cpu->sp--;

    mem->ram[cpu->sp] = (uint8_t)(value & 0xFF);
    cpu->sp--;

    return true;
}

bool stack_pop16(CPU *cpu, Memory *mem, uint16_t *value)
{
    if (cpu == NULL || mem == NULL || value == NULL)
        return false;
    
    // Two bytes must already be present on the stack.
    if (cpu->sp > RAM_SIZE - 3)
        return false;

    uint8_t low;
    uint8_t high;

    if (!stack_pop8(cpu, mem, &low))
        return false;

    if (!stack_pop8(cpu, mem, &high))
        return false;

    *value = ((uint16_t)high << 8) | low;

    return true;
}