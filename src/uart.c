// uart.c

#include "uart.h"
#include <stdio.h>

// Initialize UART with the specified baud rate
void uart_init(UART *uart)
{
    // Set baud rate: UBRR = (F_CPU / (16 * baud)) - 1
    uart->ubrr = (F_CPU / (16 * UART_BAUD)) - 1;
    uart->ucsrC = (1 << 1) | (1 << 2);  // 8-bit data
    uart->ucsrB = (1 << 3);  // Enable TX

    // Init FIFO
    uart->rx_head = 0;
    uart->rx_tail = 0;
}

// Send a byte of data through UART
void uart_send(UART *uart, uint8_t data)
{
    // Transmit data
    uart->udr = data;
    uart->ucsrA |= (1 << 5); // TX complete
}

// Receive a byte of data from UART
uint8_t uart_receive(UART *uart)
{
    if (uart->rx_head == uart->rx_tail)
        return 0xFF;  // Empty FIFO

    uint8_t data = uart->rx_buffer[uart->rx_tail];
    uart->rx_tail = (uart->rx_tail + 1) % UART_RX_BUFFER_SIZE;
    return data;
}

void uart_simulate_receive(UART *uart, uint8_t byte)
{
    uint8_t next = (uart->rx_head + 1) % UART_RX_BUFFER_SIZE;
    if (next != uart->rx_tail)  // FIFO not full
    {
        uart->rx_buffer[uart->rx_head] = byte;
        uart->rx_head = next;
    }
    else
    {
        // Overflow, drop the byte
        printf("[UART SIM] Warning: RX buffer overflow, byte 0x%02X lost\n", byte);
    }
}