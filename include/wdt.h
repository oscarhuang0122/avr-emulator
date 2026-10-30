// wdt.h - AVR Watchdog Timer Emulation
#ifndef WDT_H
#define WDT_H

#include <stdint.h>
#include "cpu.h"

// Simulated WDT register bits (based on real ATmega)
// WDE  Watchdog System Reset Enable
// WDIE Watchdog Interrupt Enable
// WDIF Watchdog Interrupt Flag

// WDTCSR bit positions
#define WDIF 7
#define WDIE 6
#define WDP3 5
#define WDCE 4
#define WDE  3
#define WDP2 2
#define WDP1 1
#define WDP0 0

#define WDTCSR_ADDR 0x60  // Typical address in I/O space (for realism)
#define WDT_TIMEOUT_CYCLES 10  // Adjustable simulation delay

typedef struct
{
    uint8_t wdtcsr;         // Control and Status Register
    uint16_t counter;       // Internal Countdown
    uint8_t interrupt_pending;
}WDT;

void wdt_init(WDT *wdt);
void wdt_write(WDT *wdt, uint8_t value);
uint8_t wdt_read(WDT *wdt);
void wdt_reset(WDT *wdt);
void wdt_step(WDT *wdt, CPU *cpu);

#endif
