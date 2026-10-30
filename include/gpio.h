// gpio.h

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Memory-mapped I/O addresses (ATmega-style)
#define PINB_ADDR 0x23
#define DDRB_ADDR 0x24
#define PORTB_ADDR 0x25

typedef struct 
{
    uint8_t ddrb;  // Direction register
    uint8_t portb;  // Output register
    uint8_t pinb;  // Input pins (can be set externally)
}GPIO;

// Initialization
void gpio_init(GPIO *gpio);

// Memory-mapped access
uint8_t gpio_read(GPIO *gpio, uint16_t addr);
void gpio_write(GPIO *gpio, uint16_t addr, uint8_t value);
int gpio_is_addr(uint16_t addr);

// Simulation step (called each cycle)
void gpio_step(GPIO *gpio);

// External control (simulate external input)
void gpio_set_input(GPIO *gpio, uint8_t value);

#endif 