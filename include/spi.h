// spi.h
#ifndef SPI_H
#define SPI_H

#include <stdint.h>
typedef struct Memory Memory;

// Example AVR-like SPI register addresses
#define SPCR_ADDR 0x4C  // SPI Control Register
#define SPSR_ADDR 0x4D  // SPI Status Register
#define SPDR_ADDR 0x4E  // SPI Data Register

// SPCR bits
#define SPIE 7  // SPI Interrupt Enable
#define SPE  6  // SPI Enable
#define DORD 5  // Data Order
#define MSTR 4  // Master/Slave Select
#define CPOL 3  // Clock Polarity
#define CPHA 2  // Clock Phase
#define SPR1 1  // Clock Rate Select 1
#define SPR0 0  // Clock Rate Select 0

// SPSR bits
#define SPIF 7  // SPI Interrupt Flag
#define WCOL 6  // Write Collision Flag

typedef struct SPI
{
    uint8_t spcr;  // Control Register
    uint8_t spsr;  // Status Register
    uint8_t spdr;  // Data Register
    uint8_t transfer_in_progress;
    uint8_t transfer_timer;

    // Master/Slave simulation fields
    struct SPI *connected_slave;  // Pointer to the other SPI (if master)
    uint8_t ss;                   // Slave select line (0 = selected)
    uint8_t mosi, miso, sck;      // Logic line simulation
}SPI;

// Function Declarations
void spi_init(SPI *spi);
void spi_connect(SPI *master, SPI *slave);
uint8_t spi_read(SPI *spi, uint16_t addr);
void spi_write(SPI *spi, uint16_t addr, uint8_t value);
void spi_step(SPI *spi, Memory *mem);
void spi_start_transfer(SPI *spi, uint8_t data);

#endif