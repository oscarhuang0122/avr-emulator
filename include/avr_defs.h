// avr_defs.h
#ifndef AVR_DEFS_H
#define AVR_DEFS_H

#define RAM_SIZE 2048
#define ROM_SIZE 32768

// CPU / Interrupt
#define SREG_ADDR   0x5F
#define EIMSK_ADDR  0x3D
#define EIFR_ADDR   0x3C

#define SPCR_ADDR 0x4C
#define SPSR_ADDR 0x4D
#define SPDR_ADDR 0x4E


#endif