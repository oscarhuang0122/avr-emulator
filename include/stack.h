// stack.h
#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"
#include "memory.h"

bool stack_push8(CPU *cpu, Memory *mem, uint8_t value);
bool stack_pop8(CPU *cpu, Memory *mem, uint8_t *value);
bool stack_push16(CPU *cpu, Memory *mem, uint16_t value);
bool stack_pop16(CPU *cpu, Memory *mem, uint16_t *value);

#endif