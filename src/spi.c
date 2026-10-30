// spi.c
#include "spi.h"
#include "interrupts.h"
#include "memory.h"
#include <stdio.h>
#include <stddef.h>

void spi_init(SPI *spi)
{
    spi->spcr = 0x00;
    spi->spsr = 0x00;
    spi->spdr = 0x00;
    spi->transfer_in_progress = 0;
    spi->transfer_timer = 0;
    spi->connected_slave = NULL;
    spi->ss = 1;  // Inactive (not selected)
    spi->mosi = 0;
    spi->miso = 0;
    spi->sck = 0;
}

void spi_connect(SPI *master, SPI *slave)
{
    master->connected_slave = slave;
    printf("[SPI] Master connected to slave.\n");
}

uint8_t spi_read(SPI *spi, uint16_t addr)
{
    switch (addr)
    {
        case SPCR_ADDR: return spi->spcr;
        case SPSR_ADDR: return spi->spsr;
        case SPDR_ADDR: return spi->spdr;
        default: return 0xFF;
    }
}

void spi_write(SPI *spi, uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case SPCR_ADDR:
            spi->spcr = value;
            break;
        case SPSR_ADDR:
            spi->spsr = value;  // Writing Clears Flags In AVR
            break;
        case SPDR_ADDR:
            spi->spdr = value;
            if (spi->spcr & (1 << SPE)) // SPI enabled
            {
                spi_start_transfer(spi, value);
            }
            break;
    }
}

void spi_start_transfer(SPI *spi, uint8_t data)
{
    spi->transfer_in_progress = 1;
    spi->transfer_timer = 8; // Arbitrary delay for simulation
    spi->spdr = data;  // store outgoing byte for reference

    printf("[SPI] Starting transfer: 0x%02X\n", data);

    // Master drives clock and MOSI line
    if (spi->spcr & (1 << MSTR))
    {
        spi->ss = 0;  // select slave

        if (spi->connected_slave)
        {
            // Pass data bit-by-bit to slave (simulated)
            spi->connected_slave->spdr = data;  // Slave receives same byte
            spi->connected_slave->transfer_in_progress = 1;
            spi->connected_slave->transfer_timer = spi->transfer_timer;
            spi->connected_slave->spsr &= ~(1 << SPIF);
        }
    }
}

void spi_step(SPI *spi, Memory *mem)
{
    if (!spi->transfer_in_progress)
        return;
    
    if (spi->transfer_timer > 0)
    {
        spi->transfer_timer--;
        return;
    }

    // Simulate data exchange
    if (spi->spcr & (1 << MSTR))
    {
        // Master receives data from slave (if connected)
        if (spi->connected_slave)
        {
            spi->spdr = spi->connected_slave->spdr;  // Receive back
        }
        else
        {
            spi->spdr = 0xFF;  // no slave + dummy
        }
    }
    else
    {
        // Slave generates response
        spi->spdr = 0xA5;  // dummy response byte
    }

    // Mark transfer done
    spi->spsr |= (1 << SPIF);
    spi->transfer_in_progress = 0;

    printf("[SPI] %s transfer complete, data=0x%02X\n", 
            (spi->spcr & (1 << MSTR)) ? "Master" : "Slave", spi->spdr);
    if (mem && (spi->spcr & (1 << SPIE)))
    {
        // Use interrupt_trigger to set a bit in mem->int_flags
        // Pick interrupt index 4 for SPI 
        interrupt_trigger(mem, 4);
    }
}