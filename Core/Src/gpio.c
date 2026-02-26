/*
 * gpio.c
 *
 *  Created on: Feb 12, 2026
 *      Author: Rubin Khadka
 */

#include "main.h"
#include "gpio.h"

void LED_Init(void)
{
  // Enable clock for GPIOC
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

  // Clear existing configuration for PC13
  GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);

  // Configure gpio as output
  GPIOC->CRH |= GPIO_CRH_MODE13;
}

void LED_ON(void)
{
  GPIOC->BSRR = GPIO_BSRR_BR13;
}

void LED_OFF(void)
{
  GPIOC->BSRR = GPIO_BSRR_BS13;
}

void LED_Toggle(void)
{
  GPIOC->ODR ^= GPIO_ODR_ODR13;
}
