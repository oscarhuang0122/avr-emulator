// program.c
#include <avr/io.h>

int main(void)
{
    DDRB = 0xF0;
    //PORTB = 0xFF;

    unsigned char value = 0x10;

    value -= 1;

    PORTB = value;

    while (1)
    {
    }

    return 0;
}

/*
    Interrupt service routine.
    Interrupt handler placed at the emulator's test vector, 0x0010.
    Writes 0x55 to PORTB and return from interrupt.
*/

void interrupt_handler(void) __attribute__((naked, used, section(".isr")));

void interrupt_handler(void)
{
    __asm__ __volatile__(
        "ldi r24, 0x55\n"
        "out 0x05, r24\n"
        "reti\n"
    );
}