// i2c.h
#ifndef I2C_H
#define I2C_H
#include <stdint.h>
typedef struct Memory Memory;

// Example AVR-like I2C (TWI) register addresses
#define TWBR_ADDR 0xB8  // Bit Rate Register
#define TWSR_ADDR 0xB9  // Status Register
#define TWAR_ADDR 0xBA  // Address Register
#define TWDR_ADDR 0xBB  // Data Register
#define TWCR_ADDR 0xBC  // Control Register

// TWCR bits
#define TWINT 7  // Interrupt Flag
#define TWEA  6  // Enable Acknowledge
#define TWSTA 5  // Start Condition
#define TWSTO 4  // Stop Condition
#define TWWC  3  // Write Collision
#define TWEN  2  // Enable
#define TWIE  0  // Interrupt Enable

// TWSR bit masks
#define TWS_MASK 0xF8  // Status bits mask (upper 5 bits)

typedef struct I2C
{
    uint8_t twbr;  // Bit Rate
    uint8_t twsr;  // Status
    uint8_t twar;  // Slave adrress
    uint8_t twdr;  // Data
    uint8_t twcr;  // Control

    uint8_t busy;  // Ongoing transaction
    uint8_t start_send;
    uint8_t stop_send;

    struct I2C *connected_slave;
}I2C;

// Function Prototypes
void i2c_init(I2C *i2c);
void i2c_connect(I2C *master, I2C *slave);
uint8_t i2c_read(I2C *i2c, uint16_t addr);
void i2c_write(I2C *i2c, uint16_t addr, uint8_t value);
void i2c_step(I2C *i2c, struct Memory *mem);

#endif