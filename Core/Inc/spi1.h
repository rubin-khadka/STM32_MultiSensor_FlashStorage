/*
 * spi1.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef SPI1_H_
#define SPI1_H_

// CS Control for W25Q64 (PB6)
#define SPI1_CS_LOW()       (GPIOB->BRR = GPIO_BRR_BR6)
#define SPI1_CS_HIGH()      (GPIOB->BSRR = GPIO_BSRR_BS6)

// Function Prototypes
void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif /* SPI1_H_ */
