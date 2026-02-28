/*
 * spi1.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"

void SPI1_Init(void)
{
  SPI1->CR1 |= SPI_CR1_SPE; // Enable SPI
}

uint8_t SPI1_Transfer(uint8_t data)
{
  // Wait for TX buffer empty
  while(!(SPI1->SR & SPI_SR_TXE));

  // Send data
  SPI1->DR = data;

  // Wait for RX buffer not empty
  while(!(SPI1->SR & SPI_SR_RXNE));

  // Read data
  uint8_t received = SPI1->DR;

  return received;
}
