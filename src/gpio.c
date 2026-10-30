// gpio.c

#include "gpio.h"

void gpio_init(GPIO *gpio)
{
    gpio->ddrb = 0;
    gpio->portb = 0;
    gpio->pinb = 0;
}

int gpio_is_addr(uint16_t addr)
{
    return (addr >= PINB_ADDR && addr <= PORTB_ADDR);
}

uint8_t gpio_read(GPIO *gpio, uint16_t addr)
{
    switch(addr)
    {
        case PORTB_ADDR: return gpio->portb;
        case DDRB_ADDR: return gpio->ddrb;
        case PINB_ADDR: return gpio->pinb;
        default: return 0xFF;
    }
}

void gpio_write(GPIO *gpio, uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case PORTB_ADDR: gpio->portb = value; break;
        case DDRB_ADDR: gpio->ddrb = value; break;
        case PINB_ADDR: 
            // Writing 1 toggles the corresponding PORTB bit.
            gpio->portb ^= value; 
            break;
    }
}

// Simulate GPIO output pin logic (update PINB from PORTB & DDRB)
void gpio_step(GPIO *gpio)
{
    // Output pins take value from PORTB
    // Input pins retain externally set value
    for (int i = 0; i < 8; i++)
    {
        if ((gpio->ddrb >> i) & 1)
        {
            // Output pin: set PINB to PORTB value
            if ((gpio->portb >> i) & 1)
                gpio->pinb |= (1 << i);
            else
                gpio->pinb &= ~(1 << i);
        }
        // else: retain external input
    }
}

// Simulate external input (e.g. button press on pin)
void gpio_set_input (GPIO *gpio, uint8_t value)
{
    // Only affects pins set as input (ddrb bit = 0)
    for (int i = 0; i < 8; i++)
    {
        if (!((gpio->ddrb >> i) & 1))
        {
            if ((value >> i) & 1)
                gpio->pinb |= (1 << i);
            else
                gpio->pinb &= ~(1 << i);
        }
    }
}