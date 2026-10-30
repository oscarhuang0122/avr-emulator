// test_stack.c
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "memory.h"
#include "stack.h"

static void setup(CPU *cpu, Memory *mem)
{
    cpu_reset(cpu);
    memory_init(mem);
    mem->cpu = cpu;
}

static void test_push_pop8(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    assert(stack_push8(&cpu, &mem, 0xAB));
    assert(cpu.sp == original_sp - 1);

    uint8_t value = 0;
    assert(stack_pop8(&cpu, &mem, &value));

    assert(value == 0xAB);
    assert(cpu.sp == original_sp);

    printf("✅ [PASS] test_push_pop8\n");
}

static void test_push_pop16(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint16_t original_sp = cpu.sp;

    assert(stack_push16(&cpu, &mem, 0x1234));
    assert(cpu.sp == original_sp - 2);

    uint16_t value = 0;
    assert(stack_pop16(&cpu, &mem, &value));

    assert(value == 0x1234);
    assert(cpu.sp == original_sp);

    printf("✅ [PASS] test_push_pop16\n");
}

static void test_stack_underflow(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    uint8_t value8 = 0;
    uint16_t value16 = 0;

    assert(!stack_pop8(&cpu, &mem, &value8));
    assert(!stack_pop16(&cpu, &mem, &value16));

    printf("✅ [PASS] test_stack_underflow\n");
}

static void test_stack_overflow(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    cpu.sp = 0;

    assert(!stack_push8(&cpu, &mem, 0x55));
    assert(!stack_push16(&cpu, &mem, 0x1234));

    printf("✅ [PASS] test_stack_overflow\n");
}

static void test_stack_lifo(void)
{
    CPU cpu;
    Memory mem;
    setup(&cpu, &mem);

    assert(stack_push8(&cpu, &mem, 0x11));
    assert(stack_push8(&cpu, &mem, 0x22));
    assert(stack_push8(&cpu, &mem, 0x33));

    uint8_t value;

    assert(stack_pop8(&cpu, &mem, &value));
    assert(value == 0x33);

    assert(stack_pop8(&cpu, &mem, &value));
    assert(value == 0x22);

    assert(stack_pop8(&cpu, &mem, &value));
    assert(value == 0x11);

    printf("✅ [PASS] test_stack_lifo\n");
}

int main(void)
{
    test_push_pop8();
    test_push_pop16();
    test_stack_underflow();
    test_stack_overflow();
    test_stack_lifo();

    printf("\n🎉 All stack tests passed!\n");
    return 0;
}