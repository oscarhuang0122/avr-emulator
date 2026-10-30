// eeprom.h
#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

#define EEPROM_SIZE 1024  // Typical 1KB EEPROM
#define EEPROM_WRITE_DELAY 4  // Simulated write delay in cycles

typedef struct
{
    uint8_t data[EEPROM_SIZE];  // Actual memory array
    uint8_t eecr;  // Control register
    uint8_t eearh; // Address high byte
    uint8_t eearl; // Address low byte
    uint8_t eedr;  // Data register
    
    // Internal simulation fields
    uint8_t write_in_progress;
    uint16_t write_timer;
}EEPROM;

void eeprom_init(EEPROM *eeprom);
void eeprom_write_byte(EEPROM *eeprom, uint16_t addr, uint8_t value);
uint8_t eeprom_read_byte(EEPROM *eeprom, uint16_t addr);
void eeprom_step(EEPROM *eeprom);  // Simulate write timing

#endif
