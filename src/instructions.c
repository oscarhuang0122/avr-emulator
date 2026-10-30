// instruction.c

#include <stdio.h>
#include "instructions.h"
#include "interrupts.h"
#include "stack.h"

#define TRACE_INSTRUCTIONS 0

#define SREG_C (1u << 0)
#define SREG_Z (1u << 1)
#define SREG_N (1u << 2)
#define SREG_V (1u << 3)
#define SREG_S (1u << 4)
#define SREG_H (1u << 5)

#define SREG_ARITH_MASK \
    (SREG_C | SREG_Z | SREG_N | SREG_V | SREG_S | SREG_H)

static void update_add_flags(CPU *cpu, uint8_t rd, uint8_t rr, uint8_t result)
{
    uint8_t rd7 = (rd >> 7) & 1u;
    uint8_t rr7 = (rr >> 7) & 1u;
    uint8_t r7  = (result >> 7) & 1u;

    uint8_t h = (((rd & 0x0F) + (rr & 0x0F)) > 0x0F);
    uint8_t v = (uint8_t)((~(rd7 ^ rr7) & (rd7 ^ r7)) & 1u);
    uint8_t n = r7;
    uint8_t s = n ^ v;
    uint8_t z = (result == 0);
    uint8_t c = (((uint16_t)rd + rr) > 0xFF);

    cpu->sreg = (uint8_t)(
        (cpu->sreg & (uint8_t)~SREG_ARITH_MASK) |
        (h ? SREG_H : 0) |
        (s ? SREG_S : 0) |
        (v ? SREG_V : 0) |
        (n ? SREG_N : 0) |
        (z ? SREG_Z : 0) |
        (c ? SREG_C : 0)
    );
}

static void update_sub_flags(CPU *cpu, uint8_t rd, uint8_t rr, uint8_t result)
{
    uint8_t rd7 = (rd >> 7) & 1u;
    uint8_t rr7 = (rr >> 7) & 1u;
    uint8_t r7  = (result >> 7) & 1u;

    uint8_t h = ((rd & 0x0F) < (rr & 0x0F));
    uint8_t v = (uint8_t)(((rd7 ^ rr7) & (rd7 ^ r7)) & 1u);
    uint8_t n = r7;
    uint8_t s = n ^ v;
    uint8_t z = (result == 0);
    uint8_t c = (rd < rr);

    cpu->sreg = (uint8_t)(
        (cpu->sreg & (uint8_t)~SREG_ARITH_MASK) |
        (h ? SREG_H : 0) |
        (s ? SREG_S : 0) |
        (v ? SREG_V : 0) |
        (n ? SREG_N : 0) |
        (z ? SREG_Z : 0) |
        (c ? SREG_C : 0)
    );
}

void execute_instruction(CPU *cpu, Memory *mem)
{
    // Check interrupt before fetching the next instruction
    if ((cpu->sreg & (1 << 7)) && mem->int_flags & (1 << 0))
    {
        // Push PC onto stack
        printf("INT PUSH PC=%04X SP=%04X\n", cpu->pc, cpu->sp);
        
        if (!stack_push16(cpu, mem, cpu->pc))
        {
            printf("Interrupt failed: stack overflow\n");
            return;
        }

        printf("SP after push=%04X\n", cpu->sp);

        cpu->pc = cpu->interrupt_vector;  // Jump to interrupt vector
        cpu->sreg &= ~(1 << 7);  // Disable global interrupts
        mem->int_flags &= ~(1 << 0);  // Clear interrupt flag
        cpu->interrupt_pending = 1;
        
        printf("[INTERRUPT] Jumping to vector 0x%04X\n", cpu->pc);
        return;
    }

    if (cpu->pc + 1 >= ROM_SIZE)
    {
        printf("PC outside ROM: %04X\n", cpu->pc);
        return;
    }

    // Fetch: Get 16-bit instruction from ROM
    uint16_t instruction_pc = cpu->pc;
    uint16_t opcode = mem->rom[cpu->pc] | (mem->rom[cpu->pc +1] << 8);
    
    // Advance to next instruction
    cpu->pc += 2;

    // NOP
    if (opcode == 0x0000)
    {
        printf("NOP\n");
        return;
    }

    // LDI: Load immediate into R16-R31
    if ((opcode & 0xF000) == 0xE000)
    {
        uint8_t d = 16 + ((opcode & 0x00F0) >> 4);

        uint8_t k = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
        
        cpu->registers[d] = k;

        printf("LDI R%d, 0x%02x\n", d, k);

        return;
    }

    // ADD: 0000 11rd dddd rrrr
    if ((opcode & 0xFC00) == 0x0C00)
    {
        uint8_t d = ((opcode >> 4) & 0x1F);
        uint8_t r = ((opcode & 0x000F) | ((opcode >> 5) & 0x10));

        uint8_t rd = cpu->registers[d];
        uint8_t rr = cpu->registers[r];
        uint8_t result = (uint8_t)(rd + rr);

        cpu->registers[d] = result;

        update_add_flags(cpu, rd, rr, result);

        printf("ADD R%d, R%d\n", d, r);
        return;
    }

    // SUB: 0001 10rd dddd rrrr
    if ((opcode & 0xFC00) == 0x1800)
    {
        uint8_t d = (opcode >> 4) & 0x1F;
        uint8_t r = (opcode & 0x000F) | ((opcode >> 5) & 0x10);

        uint8_t rd = cpu->registers[d];
        uint8_t rr = cpu->registers[r];
        uint8_t result = (uint8_t)(rd - rr);

        cpu->registers[d] = result;

        update_sub_flags(cpu, rd, rr, result);

        printf("SUB R%d, R%d\n", d, r);
        return;
    }

    // SUBI: 0101 KKKK dddd KKKK
    // SUBI: only works on registers R16-R31
    if ((opcode & 0xF000) == 0x5000)
    {
        uint8_t d = 16 + ((opcode >> 4) & 0x0F);
        uint8_t k = ((opcode >> 4) & 0xF0) | (opcode & 0x0F);

        uint8_t rd = cpu->registers[d];
        uint8_t result = (uint8_t)(rd - k);

        cpu->registers[d] = result;

        update_sub_flags(cpu, rd, k, result);

        printf("SUBI R%d, 0x%02X\n", d, k);
        return;
    }

    // RJMP: 1100 kkkk kkkk kkkk
    if ((opcode & 0xF000) == 0xC000)
    {
        int16_t k = opcode & 0x0FFF;

        // Sign extend 12-bit offset
        if (k & 0x0800)
        {
            k |= 0xF000;
        }

        // AVR relative jump offset is measured in instruction words.
        // The emulator PC uses byte addresses.
        cpu->pc += k * 2;

    #if TRACE_INSTRUCTIONS
        printf("RJMP %d\n", k);
    #endif
    
        return;
    }

    // BRNE: 1111 01kk kkkk k001
    if ((opcode & 0xFE07) == 0xF401)
    {
        int8_t k = (opcode >> 3) & 0x7F;
        if (k & 0x40) k |= 0x80;  // Sign-extend 7-bit to 8-bit

        if ((cpu->sreg & (1 << 1)) == 0)  // Z flag == 0
        {
            cpu->pc += k * 2;
            printf("BRNE %d (taken)\n", k);
        }
        else
        {
            printf("BRNE %d (not taken)\n", k);
        }
        return;
    }

    // IN: 1011 0AAd dddd AAAA
    if ((opcode & 0xF800) == 0xB000)
    {
        uint8_t d = (opcode >> 4) & 0x1F;

        uint8_t A = (opcode & 0x0F) | ((opcode >> 5) & 0x30);

        uint8_t value = mem_read(mem, A + 0x20);

        cpu->registers[d] = value;

        printf("IN R%d, 0x%02X (read 0x%02X)\n", d, A, value);
        return;
    }

    // OUT: 1011 1AAd dddd AAAA
    if ((opcode & 0xF800) == 0xB800)
    {
        uint8_t r = (opcode >> 4) & 0x1F;

        uint8_t A = (opcode & 0x0F) | ((opcode >> 5) & 0x30);

        uint8_t value = cpu->registers[r];

        mem_write(mem, A + 0x20, value);

        printf("OUT 0x%02X, R%d (write 0x%02X)\n", A, r, value);

        return;
    }

    // RETI (Return from Interrupt), AVR opcode for RETI is 0x9518 
    if (opcode == 0x9518)
    {
        if (!cpu->interrupt_pending)
        {
            printf("ERROR: RETI executed outside interrupt handler at PC=0x%04X\n",
            instruction_pc);
            return;
        }
        uint16_t addr;

        printf("RETI SP before=%04X\n", cpu->sp);

        if (!stack_pop16(cpu, mem, &addr))
        {
            printf("RETI failed: stack underflow\n");
            return;
        }        

        cpu->pc = addr;

        cpu->sreg |= (1 << 7);          // Re-enable interrupts
        cpu->interrupt_pending = 0;     // Clear interrupt state

        printf("RETI pop: SP=%04X\n", cpu->sp);
        printf("RETI -> PC=0x%04X\n", cpu->pc);
        return;
    }

    // PUSH: 1001 001d dddd 1111
    if ((opcode & 0xFE0F) == 0x920F)
    {
        uint8_t d = (opcode >> 4) & 0x1F;

        if (!stack_push8(cpu, mem, cpu->registers[d]))
        {
            printf("PUSH R%d failed: stack overflow\n", d);
            return;
        }

        printf("PUSH R%d SP=%04X\n", d, cpu->sp);
        return;
    }

    // POP: 1001 000d dddd 1111
    if ((opcode & 0xFE0F) == 0x900F)
    {
        uint8_t d = (opcode >> 4) & 0x1F;
        uint8_t value;
        
        if (!stack_pop8(cpu, mem, &value))
        {
            printf("POP R%d failed: stack underflow\n", d);
            return;
        }

        cpu->registers[d] = value;

        printf("POP R%d SP=%04X\n", d, cpu->sp);
        return;
    }

    // RCALL: 1101 kkkk kkkk kkkk
    if ((opcode & 0xF000) == 0xD000)
    {
        uint16_t ret = cpu->pc;

        if (!stack_push16(cpu, mem, ret))
        {
            printf("RCALL failed: stack overflow\n");
            return;
        }

        int16_t k = opcode & 0x0FFF;

        if (k & 0x0800)
            k |= 0xF000;

        cpu->pc += k * 2;

        printf("RCALL push return=%04X SP=%04X\n", ret, cpu->sp);
        printf("RCALL %d\n", k);
        return;
    }

    // RET: 1001 0101 0000 1000
    if (opcode == 0x9508)
    {
        uint16_t addr;

        if (!stack_pop16(cpu, mem, &addr))
        {
            printf("RET failed: stack underflow\n");
            return;
        }

        cpu->pc = addr;

        printf("RET pop SP=%04X\n", cpu->sp);
        printf("RET -> PC=0x%04X\n", cpu->pc);
        return;
    }

    // LDS: 1001 000d dddd 0000 (next word = address)
    if ((opcode & 0xFE0F) == 0x9000)
    {
        uint8_t d = (opcode >> 4) & 0x1F;

        if (cpu->pc + 1 >= ROM_SIZE)
        {
            printf("Truncated LDS/STS operand at PC=0x%04X\n", cpu->pc);
            return;
        }

        uint16_t addr = (uint16_t)mem->rom[cpu->pc] | ((uint16_t)mem->rom[cpu->pc + 1] << 8);
        cpu->pc += 2;

        cpu->registers[d] = mem_read(mem, addr);

        printf("LDS R%d, 0x%04X\n", d, addr);
        return;
    }

    // STS: 1001 001d dddd 0000
    if ((opcode & 0xFE0F) == 0x9200)
    {
        uint8_t d = (opcode >> 4) & 0x1F;

        if (cpu->pc + 1 >= ROM_SIZE)
        {
            printf("Truncated LDS/STS operand at PC=0x%04X\n", cpu->pc);
            return;
        }

        uint16_t addr = (uint16_t)mem->rom[cpu->pc] | ((uint16_t)mem->rom[cpu->pc + 1] << 8);
        cpu->pc += 2;

        mem_write(mem, addr, cpu->registers[d]);

        printf("STS 0x%04X, R%d\n", addr, d);
        return;
    }

    // Unknown
    printf("Unknown instruction: 0x%04X at PC=0x%04X\n", opcode, instruction_pc);
}