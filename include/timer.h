// timer.h

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
//struct Memory;
typedef struct Memory Memory;


// Timer0 registers (I/O mapped, typical ATmega values)
#define TCCR0A_ADDR 0x44
#define TCNT0_ADDR 0x46
#define OCR0A_ADDR 0x47
#define TIMSK0_ADDR 0x6E
#define TIFR0_ADDR 0x35

typedef struct Timer
{
    uint8_t tccr0a; // Control register
    uint8_t tcnt0;  // Counter
    uint8_t ocr0a;  // Compare match
    uint8_t timsk0; // Interrupt mask
    uint8_t tifr0;  // Interrupt flags
    uint16_t prescaler_count; // Divider for prescaler
    //Memory *mem;
} Timer;

void timer_init(Timer *timer);
void timer_step(Timer *timer, Memory *mem); // Call every CPU cycle
uint8_t timer_read(Timer *timer, uint16_t addr);
void timer_write(Timer *timer, uint16_t addr, uint8_t value);

#endif