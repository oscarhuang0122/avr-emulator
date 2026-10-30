// adc.h
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// Forward declaration
typedef struct Memory Memory;

// AVR ADC Register Addresses (example for ATmega328-like layout)
#define ADMUX_ADDR 0x7C  // ADC Multiplexer Selection Register
#define ADCSRA_ADDR 0x7A  // ADC Control and Status Register A
#define ADCL_ADDR 0x78  // ADC Data Register Low
#define ADCH_ADDR 0x79  // ADC Data Register High

#define ADC_INT_VECTOR 0x1A  // Example interrupt vector

typedef struct
{
    uint8_t admux;  // ADC Multiplexer selection register
    uint8_t adcsra;  // ADC Control and Status register (enable, start, interrupt, prescaler)
    uint16_t adc_data;  // Conversion result (10-bit)
    uint8_t conversion_in_progress;
    uint16_t conversion_timer;  // Cycles until conversion complete
}ADC;

// Initialize ADC registers
void adc_init(ADC *adc);

// Step ADC (called every CPU cycle)
void adc_step(ADC *adc, struct Memory *mem);

// Read from ADC register
uint8_t adc_read(ADC *adc, uint16_t addr);

// Write to ADC register
void adc_write(ADC *adc, uint16_t addr, uint8_t value);

#endif