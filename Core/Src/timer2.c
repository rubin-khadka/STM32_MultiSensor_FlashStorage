/*
 * timer2.c
 *
 *  Created on: Feb 20, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "timer2.h"

void TIMER2_Init(void)
{
  // Enable TIM2 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // Small delay for clock to stabilize
  for(volatile int i = 0; i < 10; i++);

  // Configure for 1ms resolution at 72MHz
  TIM2->PSC = 7200 - 1;  // Prescaler = 71999

  // Disable interrupt
  TIM2->DIER &= ~TIM_DIER_UIE;

  // Start timer
  TIM2->CR1 |= TIM_CR1_CEN;
}

void TIMER2_Delay_ms(uint16_t ms)
{
  // Stop timer if running
  TIM2->CR1 &= ~TIM_CR1_CEN;

  // Set auto-reload value
  TIM2->ARR = (uint16_t) (ms - 1) * 10;

  // Reset counter
  TIM2->CNT = 0;

  // Start timer
  TIM2->CR1 |= TIM_CR1_CEN;

  // Wait until counter reaches ARR
  while(!(TIM2->SR & TIM_SR_UIF));

  // Clear update flag
  TIM2->SR &= ~TIM_SR_UIF;

  // Stop timer
  TIM2->CR1 &= ~TIM_CR1_CEN;
}
