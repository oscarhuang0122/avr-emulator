// timer.c

#include "timer.h"
#include "interrupts.h"
#include "memory.h"
#include <stdio.h>

void timer_init(Timer *timer)
{
    timer->tccr0a = 0;
    timer->tcnt0 = 0;
    timer->ocr0a = 0;
    timer->timsk0 = 0;
    timer->tifr0 = 0;
    timer->prescaler_count = 0;
}

// Simplified prescaler: assume 1 tick = 1 CPU cycle
void timer_step(Timer *timer, Memory *mem)
{
    timer->tcnt0++;

    // check compare match
    if (timer->tcnt0 == timer->ocr0a)
    {
        timer->tifr0 |= (1 << 1); // OCF0A flag
        if (timer->timsk0 & (1 << 1)) // Interrupt enabled
        {
            interrupt_trigger(mem, 1); // Timer0 compare match interrupt
            printf("[TIMER] Compare Match -> Interrupt triggered\n");
        }
    }

    // Check overflow
    if (timer->tcnt0 == 0x00)
    {
        timer->tifr0 |= (1 << 0); // TOVO flag
        if (timer->timsk0 & (1 << 0))
        {
            interrupt_trigger(mem, 2); // Timer0 overflow interrupt
            printf("[TIMER] Overflow -> Interrupt triggered\n");
        }
    }
}

uint8_t timer_read(Timer *timer, uint16_t addr)
{
    switch(addr)
    {
        case TCCR0A_ADDR: return timer->tccr0a;
        case TCNT0_ADDR: return timer->tcnt0;
        case OCR0A_ADDR: return timer->ocr0a;
        case TIMSK0_ADDR: return timer->timsk0;
        case TIFR0_ADDR: return timer->tifr0;
        default: return 0xFF;
    }
}

void timer_write(Timer *timer, uint16_t addr, uint8_t value)
{
    switch(addr)
    {
        case TCCR0A_ADDR: timer->tccr0a = value; break;
        case TCNT0_ADDR: timer->tcnt0 = value; break;
        case OCR0A_ADDR: timer->ocr0a = value; break;
        case TIMSK0_ADDR: timer->timsk0 = value; break;
        case TIFR0_ADDR: timer->tifr0 &= (uint8_t)~value; break; // Writing clears flags
    }
}