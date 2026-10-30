// memory.c

#include <string.h>
#include "memory.h"
#include "timer.h"
#include "cpu.h"
#include "wdt.h"

void memory_init(Memory *mem)
{
    mem->int_flags = 0;
    mem->int_mask = 0;

    memset(mem->rom, 0, ROM_SIZE);
    memset(mem->ram, 0, RAM_SIZE);
    gpio_init(&mem->gpio);
    uart_init(&mem->uart);
    timer_init(&mem->timer);
    adc_init(&mem->adc);
    spi_init(&mem->spi);
    i2c_init(&mem->i2c);
    eeprom_init(&mem->eeprom);
    wdt_init(&mem->wdt);
}

uint8_t mem_read(Memory *mem, uint16_t addr)
{
    // Special registers
    if (addr == SREG_ADDR) return mem->cpu->sreg;
    if (addr == EIMSK_ADDR) return mem->int_mask;
    if (addr == EIFR_ADDR) return mem->int_flags;

    // Timer registers
    if (addr == TIFR0_ADDR || addr == TCCR0A_ADDR || addr == TCNT0_ADDR ||
        addr == OCR0A_ADDR || addr == TIMSK0_ADDR)
    {
        return timer_read(&mem->timer, addr);
    }

    // GPIO I/O
    if (gpio_is_addr(addr))
        return gpio_read(&mem->gpio, addr);

    // UART register read simulation
    if (addr == UDR0_ADDR)
        return uart_receive(&mem->uart); // Read UART data

    // ADC registers
    if (addr == ADMUX_ADDR || addr == ADCSRA_ADDR ||
        addr == ADCL_ADDR  || addr == ADCH_ADDR)
    {
        return adc_read(&mem->adc, addr);
    }

    // SPI registers
    if (addr == SPCR_ADDR || addr == SPSR_ADDR || addr == SPDR_ADDR)
    {
        return spi_read(&mem->spi, addr);
    }

    // I2C registers
    if (addr == TWBR_ADDR || addr == TWSR_ADDR || 
        addr == TWAR_ADDR || addr == TWDR_ADDR || addr == TWCR_ADDR)
    {
        return i2c_read(&mem->i2c, addr);
    }

    // Watchdog register
    if (addr == WDTCSR_ADDR)
    {
        return wdt_read(&mem->wdt);
    }

    // RAM fallback
    if (addr < RAM_SIZE)
        return mem->ram[addr];
    
    return 0xFF;
}

void mem_write(Memory *mem, uint16_t addr, uint8_t value)
{
    // Special registers
    if (addr == SREG_ADDR) { mem->cpu->sreg = value; return; }
    if (addr == EIMSK_ADDR) { mem->int_mask = value; return; }
    if (addr == EIFR_ADDR) { mem->int_flags = value; return; }

    // Timer registers
    if (addr == TIFR0_ADDR || addr == TCCR0A_ADDR || addr == TCNT0_ADDR ||
        addr == OCR0A_ADDR || addr == TIMSK0_ADDR)
    {
        timer_write(&mem->timer, addr, value);
        return;
    }

    // GPIO I/O
    if (gpio_is_addr(addr))
    {
        gpio_write(&mem->gpio, addr, value);
        return;
    }

    // UART register write simulation
    if (addr == UDR0_ADDR)
    {
        uart_send(&mem->uart, value); // Write UART data
        return;
    }

    // ADC registers
    if (addr == ADMUX_ADDR || addr == ADCSRA_ADDR ||
        addr == ADCL_ADDR  || addr == ADCH_ADDR)
    {
        adc_write(&mem->adc, addr, value);
        return;
    }
    
    // SPI registers
    if (addr == SPCR_ADDR || addr == SPSR_ADDR || addr == SPDR_ADDR)
    {
        spi_write(&mem->spi, addr, value);
        return;
    }
    
    // I2C registers
    if (addr == TWBR_ADDR || addr == TWSR_ADDR || 
        addr == TWAR_ADDR || addr == TWDR_ADDR || addr == TWCR_ADDR)
    {
        i2c_write(&mem->i2c, addr, value);
        return;
    }

    // Watchdog register
    if (addr == WDTCSR_ADDR)
    {
        wdt_write(&mem->wdt, value);
        return;
    }

    // RAM fallback
    if (addr < RAM_SIZE)
        mem->ram[addr] = value;
}