/*
 * spi1.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"

void SPI1_Init(void)
{
  // Enable clock and spi peripheral
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_SPI1EN;

  // Remap spi1 to PB3/4/5 and PA15
  AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;

  // Configure pins for connection with shift registers
  // PB5 MOSI
  GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
  GPIOB->CRL |= GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1; // 50 MHZ
  GPIOB->CRL |= GPIO_CRL_CNF5_1;  // Alternate function output push-pull

  // PB4 MISO - Input floating
  GPIOB->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
  GPIOB->CRL |= GPIO_CRL_CNF4_0;  // Input floating

  // PB3 SCK
  GPIOB->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
  GPIOB->CRL |= GPIO_CRL_MODE3_0 | GPIO_CRL_MODE3_1; // 50 MHZ
  GPIOB->CRL |= GPIO_CRL_CNF3_1;  // Alternate function output push-pull

  // PB6 CS
  GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
  GPIOB->CRL |= GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1; // 50 MHz output push pull
  GPIOB->BSRR |= GPIO_BSRR_BS6; // Start high

  // SPI1 Configurations
  SPI1->CR1 = 0;
  SPI1->CR1 |= SPI_CR1_SSM;
  SPI1->CR1 |= SPI_CR1_SSI;
  SPI1->CR1 |= SPI_CR1_MSTR;

  // 72MHz/8 - 9 MHZ
  SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;

  // CPOL and CPHA for 74HC595
  SPI1->CR1 &= ~SPI_CR1_CPOL; // Clock idle low (0)
  SPI1->CR1 &= ~SPI_CR1_CPHA; // Data capture on first edge

  // MSB first
  SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

  // Enable SPI
  SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t SPI1_Transmit(uint8_t data)
{
    uint32_t timeout = 10000;

    // Wait for TX buffer empty with timeout
    while(!(SPI1->SR & SPI_SR_TXE))
    {
        if(--timeout == 0) return 0xFF;
    }

    // Send data
    SPI1->DR = data;

    // Wait for RX buffer not empty with timeout
    timeout = 10000;
    while(!(SPI1->SR & SPI_SR_RXNE))
    {
        if(--timeout == 0) return 0xFF;
    }

    uint8_t received = SPI1->DR;

    // CRITICAL: Wait for SPI to not be busy
    timeout = 10000;
    while(SPI1->SR & SPI_SR_BSY)
    {
        if(--timeout == 0) return 0xFF;
    }

    return received;
}
