// test_gpio.c

#include <stdio.h>
#include <assert.h>
#include "gpio.h"

void test_gpio_init()
{
    GPIO gpio;
    gpio_init(&gpio);
    assert(gpio.ddrb == 0);
    assert(gpio.pinb == 0);
    assert(gpio.portb == 0);
    printf("✅ test_gpio_init passed\n");
}

void test_gpio_direction_output()
{
    GPIO gpio;
    gpio_init(&gpio);

    gpio_write(&gpio, DDRB_ADDR, 0xFF);  // all outputs
    assert(gpio.ddrb == 0xFF);

    gpio_write(&gpio, PORTB_ADDR, 0xAA);  // alternating pattern
    assert(gpio.portb == 0xAA);

    gpio_step(&gpio);
    assert(gpio.pinb ==0xAA);  // output pins reflect PORTB
    printf("✅ test_gpio_direction_output passed\n");
}

void test_gpio_direction_input()
{
    GPIO gpio;
    gpio_init(&gpio);

    gpio_write(&gpio, DDRB_ADDR, 0x00);  // all inputs
    gpio_set_input(&gpio, 0xF0);    // Simulate external input

    gpio_step(&gpio);
    assert(gpio.pinb == 0xF0);  // Should retain external input
    printf("✅ test_gpio_direction_input passed\n");
}

void test_gpio_mixed_direction()
{
    GPIO gpio;
    gpio_init(&gpio);

    gpio_write(&gpio, DDRB_ADDR, 0xF0); // upper 4 output, lower 4 input
    gpio_write(&gpio, PORTB_ADDR, 0xAA); // alternationg bits (10101010)

    gpio_set_input(&gpio, 0xFF);  // Simulate input: 00001111
    gpio_step(&gpio);

    // Expected pinb: upper 4 from PORTB (0xA0), lower 4 from input (0x0F)
    assert(gpio.pinb == (0xA0 | 0x0F));  // Should be 0xAF
    printf("✅ test_gpio_mixed_direction passed\n");
}

void test_gpio_read_write()
{
    GPIO gpio;
    gpio_init(&gpio);

    gpio_write(&gpio, DDRB_ADDR, 0x55);
    gpio_write(&gpio, PORTB_ADDR, 0xAA);
    gpio_set_input(&gpio, 0x33);
    gpio_step(&gpio);

    assert(gpio_read(&gpio, DDRB_ADDR) == 0x55);
    assert(gpio_read(&gpio, PORTB_ADDR) == 0xAA);
    assert(gpio_read(&gpio, PINB_ADDR) == gpio.pinb);
    printf("✅ test_gpio_read_write passed\n");
}

void test_gpio_pin_toggle(void)
{
    GPIO gpio;
    gpio_init(&gpio);

    gpio_write(&gpio, DDRB_ADDR, 0xFF);
    gpio_write(&gpio, PORTB_ADDR, 0xAA);

    // Toggle bits 0 and 1 through PINB.
    gpio_write(&gpio, PINB_ADDR, 0x03);
    gpio_step(&gpio);

    assert(gpio.portb == 0xA9);
    assert(gpio.pinb == 0xA9);

    printf("✅ test_gpio_pin_toggle passed\n");
}

int main()
{
    test_gpio_init();
    test_gpio_direction_output();
    test_gpio_direction_input();
    test_gpio_mixed_direction();
    test_gpio_read_write();
    test_gpio_pin_toggle();

    printf("\n🎉 All GPIO tests passed!\n");
    return 0;
}