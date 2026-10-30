// i2c.c
#include "i2c.h"
#include "interrupts.h"
#include "memory.h"
#include <stdio.h>
#include <stddef.h>

void i2c_init(I2C *i2c)
{
    i2c->twbr = 0;
    i2c->twsr = 0;
    i2c->twar = 0;
    i2c->twdr = 0xFF;
    i2c->twcr = 0;
    i2c->busy = 0;
    i2c->start_send = 0;
    i2c->stop_send = 0;
    i2c->connected_slave = NULL;
}

void i2c_connect(I2C *master, I2C *slave)
{
    master->connected_slave = slave;
    printf("[I2C] Master connected to slave.\n");
}

uint8_t i2c_read(I2C *i2c, uint16_t addr)
{
    switch (addr)
    {
        case TWBR_ADDR: return i2c->twbr;
        case TWSR_ADDR: return i2c->twsr;
        case TWAR_ADDR: return i2c->twar;
        case TWDR_ADDR: return i2c->twdr;
        case TWCR_ADDR: return i2c->twcr;
        default: return 0xFF;
    }
}

void i2c_write(I2C *i2c, uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case TWBR_ADDR:
            i2c->twbr = value;
            break;
        case TWSR_ADDR:
            i2c->twsr = value & ~TWS_MASK;  // Preserve status bits
            break;
        case TWAR_ADDR:
            i2c->twar = value;
            break;
        case TWDR_ADDR:
            i2c->twdr = value;
            break;
        case TWCR_ADDR:
            i2c->twcr = value;
            if (value & (1 << TWSTA))
            {
                // Start condition
                i2c->start_send = 1;
                i2c->busy = 1;
                i2c->twsr = 0x08;  // Start condition transmitted
                printf("[I2C] START condition issued\n");
            }
            else if (value & (1 << TWSTO))
            {
                // Stop condition
                i2c->stop_send = 1;
                i2c->busy = 0;
                i2c->twsr = 0xF8;  // Bus idle
                printf("[I2C] STOP condition issued\n");
            }
            else if (i2c->busy && (value & (1 << TWEN)))
            {
                // Transmit byte to slave
                if (i2c->connected_slave)
                {
                    i2c->connected_slave->twdr = i2c->twdr;
                    printf("[I2C] Master wrote byte 0x%02X to slave\n", i2c->twdr);
                }
                else
                {
                    printf("[I2C] No slave connected, data=0x%02X\n", i2c->twdr);
                }
                i2c->twsr = 0x28;  // Data transmitted and ACK received
            }
            break;
    }
}

void i2c_step(I2C *i2c, struct Memory *mem)
{
    if (!(i2c->twcr & (1 << TWEN)))
        return;
    
    // Simulate one transaction step (ackowledge or idle)
    if (i2c->busy && !(i2c->twcr & (1 << TWINT)))
    {
        i2c->twcr |= (1 << TWINT);  // Set interrupt flag
        printf("[I2C] Operation complete, TWINT set.\n");

        if (mem && (i2c->twcr & (1 << TWIE)))
        {
            // Trigger I2C interrupt (e.g., interrupt vector index 5)
            interrupt_trigger(mem, 5);
        }
    }
}