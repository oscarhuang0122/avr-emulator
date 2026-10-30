// uart.h

#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART_BAUD 9600
#define F_CPU 16000000  // Assuming a 16 MHZ clock
#define UART_RX_BUFFER_SIZE 16  // FIFO size

#define UDR0_ADDR 0xC6

// UART registers for a single UART channel
typedef struct 
{
    uint8_t udr;   // UART Data Register
    uint8_t ucsrA; // UART Control and Status Register A
    uint8_t ucsrB; // UART Control and Status Register B
    uint8_t ucsrC; // UART Control and Status Register C
    uint16_t ubrr; // UART Baud Rate Register

    // Receive FIFO
    uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
    uint8_t rx_head;
    uint8_t rx_tail;
}UART;

void uart_init(UART *uart);
void uart_send(UART *uart, uint8_t data);
uint8_t uart_receive(UART *uart);
void uart_simulate_receive(UART *uart, uint8_t byte);

#endif