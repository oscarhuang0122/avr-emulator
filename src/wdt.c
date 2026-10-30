// wdt.c - AVR Watchdog Timer Emulation
#include "wdt.h"
#include <stdio.h>
#include <stddef.h>

void  wdt_init(WDT *wdt)
{
    wdt->wdtcsr = 0x00;
    wdt->counter = WDT_TIMEOUT_CYCLES;
    wdt->interrupt_pending = 0;
}

void wdt_write(WDT *wdt, uint8_t value)
{
    wdt->wdtcsr = value;
    printf("[WDT] WDTCSR updated: 0x%02X\n", value);
}

uint8_t wdt_read(WDT *wdt)
{
    return wdt->wdtcsr;
}

// Reset watchdog timer manually
void wdt_reset(WDT *wdt)
{
    wdt->counter = WDT_TIMEOUT_CYCLES;
    printf("[WDT] Reset (kick the dog)\n");
}

// Called once per CPU cycle
void wdt_step(WDT *wdt, CPU *cpu)
{
    if (wdt == NULL || cpu == NULL)
        return;

    // Watchdog disabled
    if (!(wdt->wdtcsr & ((1 << WDE) | (1 << WDIE))))
        return;

    // Count down
    if (wdt->counter > 0)
    {
        wdt->counter--;
        return;
    }

    // Watchdog Timeout
    if (wdt->wdtcsr & (1 << WDIE))
    {
        // Interrupt mode
        wdt->wdtcsr |= (1 << WDIF);  // Set interrupt flag
        wdt->interrupt_pending = 1;

        printf("[WDT] Interrupt triggered!\n");
    }

    if (wdt->wdtcsr & (1 << WDE))
    {
        // Reset mode
        printf("[WDT] System reset!\n");

        cpu_reset(cpu);

        // Reset watchdog counter
        wdt->counter = WDT_TIMEOUT_CYCLES;

        return;
    }

    // Restart timer for interrupt-only mode
    wdt->counter = WDT_TIMEOUT_CYCLES;
}