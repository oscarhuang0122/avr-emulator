// test_instruction.c
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "cpu.h"
#include "memory.h"
#include "instructions.h"
#include "gpio.h"

static void setup(CPU *cpu, Memory *mem)
{
    cpu_reset(cpu);
    memory_init(mem);
    mem->cpu = cpu;
}

static void write_opcode(Memory *mem, uint16_t addr, uint16_t opcode)
{
    assert((uint32_t)addr + 1 < ROM_SIZE);

    mem->rom[addr] = (uint8_t)(opcode & 0xFF);
    mem->rom[addr + 1] = (uint8_t)(opcode >> 8);
}

static void test_ldi(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // LDI R16, 0xAB
    write_opcode(&mem, 0, 0xEA0B);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0xAB);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_ldi\n");
}

static void test_rjmp(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // RJMP +1 : skip one 16-bit instruction
    write_opcode(&mem, 0, 0xC001);

    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 4);

    // RJMP -1 : branch itself
    cpu.pc = 4;
    write_opcode(&mem, 4, 0xCFFF);

    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 4);

    printf("✅ [PASS] test_rjmp\n");
}

static void test_out(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0xF0;

    // OUT DDRB, R16 (I/O address 0x04)
    write_opcode(&mem, 0, 0xB904);

    execute_instruction(&cpu, &mem);

    assert(mem.gpio.ddrb == 0xF0);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_out\n");
}

static void test_in(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    mem.gpio.ddrb = 0xF0;
    mem.gpio.portb = 0xA0;
    gpio_step(&mem.gpio);

    // IN R18, PINB (I/O address 0x03)
    write_opcode(&mem, 0, 0xB123);

    execute_instruction(&cpu, &mem);
    
    assert(cpu.registers[18] == 0xA0);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_in\n");
}

static void test_add(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x12;
    cpu.registers[17] = 0x34;

    // ADD R16, R17
    write_opcode(&mem, 0, 0x0F01);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x46);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_add\n");
}

static void test_sub(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x50;
    cpu.registers[17] = 0x20;

    // SUB R16, R17
    write_opcode(&mem, 0, 0x1B01);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x30);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_sub\n");
}

static void test_subi(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x10;

    // SUBI R16, 0x01
    write_opcode(&mem, 0, 0x5001);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x0F);
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_subi\n");
}

static void test_sub_zero_flag(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x42;
    cpu.registers[17] = 0x42;

    // SUB R16, R17
    write_opcode(&mem, 0, 0x1B01);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x00);

    // AVR SREG bit 1 is the zero flag.
    assert((cpu.sreg & (1 << 1)) != 0);

    assert(cpu.pc == 2);

    printf("✅ [PASS] test_sub_zero_flag\n");
}

static void test_brne_taken(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // Z = 0: BRNE should branch.
    cpu.sreg &= ~(1 << 1);

    // BRNE +1: skip the next instruction.
    write_opcode(&mem, 0, 0xF409);

    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 4);

    printf("✅ [PASS] test_brne_taken\n");
}

static void test_brne_not_taken(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // Z = 1: BRNE should not branch.
    cpu.sreg |= (1 << 1);
    
    write_opcode(&mem, 0, 0xF409);

    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 2);

    printf("✅ [PASS] test_brne_not_taken\n");
}

static void test_push_instruction(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0xAB;
    uint16_t original_sp = cpu.sp;

    // PUSH R16
    write_opcode(&mem, 0, 0x930F);

    // Execute the instruction stored in ROM.
    execute_instruction(&cpu, &mem);

    // Verify stack pointer moved down by one byte.
    assert(cpu.sp == original_sp - 1);

    // Verify R16 was written to the stack.
    assert(mem.ram[original_sp] == 0xAB);

    // Verify PC advanced by one 16-bit instruction.
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_push_instruction\n");
}

static void test_pop_instruction(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    // PUSH R16
    cpu.registers[16] = 0xCD;
    write_opcode(&mem, 0, 0x930F);

    // POP R17
    write_opcode(&mem, 2, 0x911F);

    execute_instruction(&cpu, &mem);

    assert(cpu.sp == original_sp - 1);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[17] == 0xCD);
    assert(cpu.sp == original_sp);
    assert(cpu.pc == 4);

    printf("✅ [PASS] test_pop_instruction\n");
}

static void test_rcall_ret(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    // Address 0x0000: RCALL +1
    // Address 0x0002: NOP
    // Address 0x0004: RET
    write_opcode(&mem, 0, 0xD001);
    write_opcode(&mem, 2, 0x0000);
    write_opcode(&mem, 4, 0x9508);

    // Execute RCALL.
    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 4);
    assert(cpu.sp == original_sp - 2);

    // Execute RET.
    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 2);
    assert(cpu.sp == original_sp);

    printf("✅ [PASS] test_rcall_ret\n");
}

static void test_interrupt_entry(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    cpu.pc = 0x0004;
    cpu.interrupt_vector = 0x0010;

    // Enable global interrupts (SREG I bit)
    cpu.sreg |= (1 << 7);

    // Raise interrupt 0.
    mem.int_flags |= (1 << 0);

    execute_instruction(&cpu, &mem);

    // CPU jumps to interrupt vector.
    assert(cpu.pc == 0x0010);

    // Return address occupies two stack bytes.
    assert(cpu.sp == original_sp - 2);

    // Global interrupts disabled during ISR.
    assert((cpu.sreg & (1 << 7)) == 0);

    // Interrupt request acknowledged.
    assert((mem.int_flags & (1 << 0)) == 0);

    // Emulator tracks active interrupt.
    assert(cpu.interrupt_pending == 1);

    printf("✅ [PASS] test_interrupt_entry\n");
}

static void test_reti(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    cpu.pc = 0x0004;
    cpu.interrupt_vector = 0x0010;

    // RETI at interrupt vector.
    write_opcode(&mem, 0x0010, 0x9518);

    // Enable interrupts and request one.
    cpu.sreg |= (1 << 7);
    mem.int_flags |= (1 << 0);

    // Enter interrupt.
    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 0x0010);
    assert(cpu.sp == original_sp - 2);
    assert(cpu.interrupt_pending == 1);

    // Execute RETI.
    execute_instruction(&cpu, &mem);

    // Resume interrupted instruction.
    assert(cpu.pc == 0x0004);

    // Stack restored.
    assert(cpu.sp == original_sp);

    // RETI re-enables global interrupts.
    assert((cpu.sreg & (1 << 7)) != 0);

    // No longer inside interrupt handler.
    assert(cpu.interrupt_pending == 0);

    printf("✅ [PASS] test_reti\n");
}

static void test_interrupt_disabled(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    cpu.interrupt_vector = 0x0010;

    // Keep global interrupts disabled.
    cpu.sreg &= ~(1 << 7);

    // Interrupt request exists.
    mem.int_flags |= (1 << 0);

    // Execute NOP at address 0.
    write_opcode(&mem, 0, 0x0000);

    execute_instruction(&cpu, &mem);

    // NOP executed normally.
    assert(cpu.pc == 2);

    // No stack push occurred.
    assert(cpu.sp == original_sp);

    // Request remains pending.
    assert((mem.int_flags & (1 << 0)) != 0);

    assert(cpu.interrupt_pending == 0);

    printf("✅ [PASS] test_interrupt_disabled\n");
}

static void test_add_carry_flags(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0xFF;
    cpu.registers[17] = 0x01;

    // ADD R16, R17
    write_opcode(&mem, 0, 0x0F01);
    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x00);
    assert((cpu.sreg & (1u << 0)) != 0); // C
    assert((cpu.sreg & (1u << 1)) != 0); // Z
    assert((cpu.sreg & (1u << 5)) != 0); // H
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_add_carry_flags\n");
}

static void test_add_overflow_flags(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x7F;
    cpu.registers[17] = 0x01;

    // ADD R16, R17
    write_opcode(&mem, 0, 0x0F01);
    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0x80);
    assert((cpu.sreg & (1u << 3)) != 0); // V
    assert((cpu.sreg & (1u << 2)) != 0); // N
    assert((cpu.sreg & (1u << 4)) == 0); // S = N XOR V
    assert((cpu.sreg & (1u << 0)) == 0); // C
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_add_overflow_flags\n");
}

static void test_sub_borrow_flags(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x00;
    cpu.registers[17] = 0x01;

    // SUB R16, R17
    write_opcode(&mem, 0, 0x1B01);
    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0xFF);
    assert((cpu.sreg & (1u << 0)) != 0); // C: borrow
    assert((cpu.sreg & (1u << 5)) != 0); // H
    assert((cpu.sreg & (1u << 2)) != 0); // N
    assert((cpu.sreg & (1u << 1)) == 0); // Z
    assert(cpu.pc == 2);

    printf("✅ [PASS] test_sub_borrow_flags\n");
}

static void test_lds(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // Place a known value in SRAM.
    mem_write(&mem, 0x0100, 0xAB);

    // LDS R16, 0x0100
    // First word: opcode
    // Second word: 16-bit data address
    write_opcode(&mem, 0, 0x9100);
    write_opcode(&mem, 2, 0x0100);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[16] == 0xAB);

    // LDS occupies four bytes.
    assert(cpu.pc == 4);

    printf("✅ [PASS] test_lds\n");
}

static void test_sts(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0xCD;

    // STS 0x0100, R16
    write_opcode(&mem, 0, 0x9300);
    write_opcode(&mem, 2, 0x0100);

    execute_instruction(&cpu, &mem);

    assert(mem_read(&mem, 0x0100) == 0xCD);
    assert(cpu.pc == 4);

    printf("✅ [PASS] test_sts\n");
}

static void test_sts_lds_roundtrip(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.registers[16] = 0x5A;

    // 0x0000: STS 0x0100, R16
    write_opcode(&mem, 0, 0x9300);
    write_opcode(&mem, 2, 0x0100);

    // 0x0004: LDS R17, 0x0100
    write_opcode(&mem, 4, 0x9110);
    write_opcode(&mem, 6, 0x0100);

    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 4);
    assert(mem_read(&mem, 0x0100) == 0x5A);

    execute_instruction(&cpu, &mem);

    assert(cpu.registers[17] == 0x5A);
    assert(cpu.pc == 8);

    printf("✅ [PASS] test_sts_lds_roundtrip\n");
}

static void test_unknown_instruction(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // An opcode not currently implemented by the emulator.
    write_opcode(&mem, 0, 0xFFFF);

    execute_instruction(&cpu, &mem);

    // Unknown instructions should not corrupt CPU state.
    assert(cpu.pc == 2);
    assert(cpu.sp == RAM_SIZE - 1);

    printf("✅ [PASS] test_unknown_instruction\n");
}

static void test_truncated_lds(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    // Put LDS at the final complete instruction word in ROM.
    uint16_t addr = ROM_SIZE - 2;
    cpu.pc = addr;
    write_opcode(&mem, addr, 0x9100);

    execute_instruction(&cpu, &mem);

    // The operand is missing, so LDS must not read a fake address.
    assert(cpu.registers[16] == 0);
    assert(cpu.pc == addr + 2);

    printf("✅ [PASS] test_truncated_lds\n");
}

static void test_program_interrupt_gpio_integration(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    /*
     * Simulated AVR program:
     *
     * 0x0000: LDI R16, 0xF0
     * 0x0002: OUT DDRB, R16
     * 0x0004: LDI R17, 0xA0
     * 0x0006: OUT PORTB, R17
     * 0x0008: IN R18, PINB
     * 0x000A: RJMP -1
     *
     * Interrupt handler:
     * 0x0010: RETI
     */

    write_opcode(&mem, 0x0000, 0xEF00);
    write_opcode(&mem, 0x0002, 0xB904);
    write_opcode(&mem, 0x0004, 0xEA10);
    write_opcode(&mem, 0x0006, 0xB915);
    write_opcode(&mem, 0x0008, 0xB123);
    write_opcode(&mem, 0x000A, 0xCFFF);

    write_opcode(&mem, 0x0010, 0x9518);

    cpu.interrupt_vector = 0x0010;

    // Start with no pending interrupt requests.
    mem.int_flags = 0;

    // Enable global interrupts.
    cpu.sreg |= (1u << 7);

    // Step 1: Configure GPIO direction.
    execute_instruction(&cpu, &mem);  // LDI
    execute_instruction(&cpu, &mem);  // OUT DDRB

    assert(cpu.registers[16] == 0xF0);
    assert(mem.gpio.ddrb == 0xF0);
    assert(cpu.pc == 0x0004);

    // Step 2: Write GPIO output.
    execute_instruction(&cpu, &mem);  // LDI
    execute_instruction(&cpu, &mem);  // OUT PORTB

    assert(cpu.registers[17] == 0xA0);
    assert(mem.gpio.portb == 0xA0);
    assert(cpu.pc == 0x0008);

    // Update simulated GPIO pin state.
    gpio_step(&mem.gpio);

    // Step 3: Trigger interrupt before IN executes.
    mem.int_flags |= (1u << 0);

    execute_instruction(&cpu, &mem);

    // Interrupt entry saves the return address.
    assert(cpu.sp == original_sp - 2);
    assert(cpu.interrupt_pending == 1);
    assert((cpu.sreg & (1u << 7)) == 0);
    assert((mem.int_flags & (1u << 0)) == 0);

    // Interrupt entry should stop at the ISR vector.
    assert(cpu.pc == 0x0010);

    // Execute RETI in the next instruction step.
    execute_instruction(&cpu, &mem);

    // Step 4: Verify return from interrupt.
    assert(cpu.pc == 0x0008);
    assert(cpu.sp == original_sp);
    assert(cpu.interrupt_pending == 0);
    assert((cpu.sreg & (1u << 7)) != 0);

    // Step 5: Resume the interrupted IN instruction.
    execute_instruction(&cpu, &mem);

    assert(cpu.registers[18] == 0xA0);
    assert(cpu.pc == 0x000A);

    // Step 6: Verify the infinite loop stays at 0x000A.
    execute_instruction(&cpu, &mem);

    assert(cpu.pc == 0x000A);
    assert(cpu.sp == original_sp);

    printf("✅ [PASS] test_program_interrupt_gpio_integration\n");
}

int main(void)
{
    test_ldi();
    test_rjmp();
    test_out();
    test_in();

    test_add();
    test_sub();
    test_subi();
    test_sub_zero_flag();

    test_brne_taken();
    test_brne_not_taken();

    test_push_instruction();
    test_pop_instruction();

    test_rcall_ret();

    test_interrupt_entry();
    test_reti();
    test_interrupt_disabled();

    test_add_carry_flags();
    test_add_overflow_flags();
    test_sub_borrow_flags();

    test_lds();
    test_sts();
    test_sts_lds_roundtrip();

    test_unknown_instruction();
    test_truncated_lds();

    test_program_interrupt_gpio_integration();

    printf("\n🎉 All instruction tests passed!\n");
    return 0;
}