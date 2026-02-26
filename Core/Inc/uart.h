/*
 * uart.h
 *
 *  Created on: Feb 18, 2026
 *      Author: Rubin Khadka
 */

#ifndef UART_H
#define UART_H

#include "stdint.h"
#include "stdbool.h"

// Buffer structure
typedef struct
{
  uint8_t *buffer;
  uint16_t size;
  volatile uint16_t head;
  volatile uint16_t tail;
  volatile uint16_t count;
} USART1_Buffer_t;

/* External declarations */
extern volatile USART1_Buffer_t usart1_rx_buf;
extern volatile USART1_Buffer_t usart1_tx_buf;

// Function declarations
void USART1_Init(void);
void UART1_BufferInit(volatile USART1_Buffer_t *buff, uint8_t *storage, uint16_t size);

// ONE set of buffer functions that work with ANY buffer (using pointers)
bool USART1_BufferEmpty(volatile USART1_Buffer_t *buff);
bool USART1_BufferFull(volatile USART1_Buffer_t *buff);
bool USART1_BufferWrite(volatile USART1_Buffer_t *buff, uint8_t data);
uint8_t USART1_BufferRead(volatile USART1_Buffer_t *buff);

// High-level functions (these will use the buffer functions)
void USART1_SendChar(char c);
void USART1_SendString(char *str);
uint8_t USART1_GetChar(void);  // Get a character from RX buffer
bool USART1_DataAvailable(void);  // Check if RX data is available

void USART1_SendNumber(uint32_t num);

// Interrupt handler
void USART1_IRQHandler(void);

#endif /* UART_H */
