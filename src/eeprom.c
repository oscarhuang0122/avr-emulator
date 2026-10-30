// eeprom.c
#include "eeprom.h"
#include <stdio.h>
#include <string.h>

void eeprom_init(EEPROM *eeprom)
{
    memset(eeprom->data, 0xFF, EEPROM_SIZE);  // Default EEPROM is all 0xFF
    eeprom->eecr = 0;
    eeprom->eedr = 0;
    eeprom->write_in_progress = 0;
    eeprom->write_timer = 0;
}

uint8_t eeprom_read_byte(EEPROM *eeprom, uint16_t addr)
{
    if (addr >= EEPROM_SIZE)
    {
        printf("[EEPROM] Read out of range: 0x%04X\n", addr);
        return 0xFF;
    }

    uint8_t val = eeprom->data[addr];
    printf("[EEPROM] Read [0x%04X] = 0x%02X\n", addr, val);
    return val;
}

void eeprom_write_byte(EEPROM *eeprom, uint16_t addr, uint8_t value)
{
    if (addr >= EEPROM_SIZE)
    {
        printf("[EEPROM] Write out of range: 0x%04X\n", addr);
        return;
    }

    if (eeprom->write_in_progress)
    {
        printf("[EEPROM] Busy! Ignoring write to 0x%04X\n", addr);
        return;
    }

    eeprom->eedr = value;
    eeprom->eearl = addr & 0xFF;
    eeprom->eearh = (addr >> 8) & 0xFF;

    eeprom->write_in_progress = 1;
    eeprom->write_timer = EEPROM_WRITE_DELAY;

    printf("[EEPROM] Write scheduled: [0x%04X] = 0x%02X\n", addr, value);
}

// Simulate one CPU cycle
void eeprom_step(EEPROM *eeprom)
{
    if (eeprom->write_in_progress && eeprom->write_timer > 0)
    {
        eeprom->write_timer--;

        if (eeprom->write_timer == 0)
        {
            uint16_t addr = (eeprom->eearh << 8) | eeprom->eearl;
            eeprom->data[addr] = eeprom->eedr;
            eeprom->write_in_progress = 0;
            printf("[EEPROM] Write complete: [0x%04X] = 0x%02X\n", addr, eeprom->eedr);
        }
    }
}