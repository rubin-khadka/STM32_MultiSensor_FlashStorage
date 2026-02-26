/*
 * button.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "button.h"
#include "uart.h"

// Current display mode
static volatile DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

void Button_Init(void)
{
  // Enable Clocks
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

  // GPIO Configuration for button
  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
  GPIOA->CRL |= GPIO_CRL_CNF0_1;  // Input mode push-pull
  GPIOA->ODR |= GPIO_ODR_ODR0;    // GPIO pull-up

  // Connect PA0 to External Interrupt 0
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;

  // Disable interrupt while configuring
  EXTI->IMR &= ~EXTI_IMR_MR0;

  // Configure trigger edge
  EXTI->FTSR |= EXTI_FTSR_TR0;   // Enable falling edge trigger
  EXTI->RTSR &= ~EXTI_RTSR_TR0;  // Disable rising edge

  // Clear any pending interrupt
  EXTI->PR |= EXTI_PR_PR0;

  // Enable interrupt
  EXTI->IMR |= EXTI_IMR_MR0;

  NVIC_EnableIRQ(EXTI0_IRQn);
}

DisplayMode_t Button_GetMode(void)
{
  return current_mode;
}

// Change to next mode
void Button_NextMode(void)
{
  current_mode++;
  if(current_mode >= DISPLAY_MODE_COUNT)
  {
    current_mode = DISPLAY_MODE_TEMP_HUM;
  }

  // Debug message
  USART1_SendString("Mode changed to: ");
  switch(current_mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      USART1_SendString("Temperature/Humidity\r\n");
      break;
    case DISPLAY_MODE_ACCEL:
      USART1_SendString("Accelerometer\r\n");
      break;
    case DISPLAY_MODE_GYRO:
      USART1_SendString("Gyroscope\r\n");
      break;
    default:
      break;
  }
}

void TIMER4_Init(void)
{
  // Enable TIM4 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // Small delay for clock to stabilize
  for(volatile int i = 0; i < 10; i++);

  // Configure for 0.1ms resolution at 72MHz
  TIM4->PSC = 7200 - 1;     // Prescaler = 7199
  TIM4->ARR = 500 - 1;      // 500 ticks = 50ms

  // Enable interrupt
  TIM4->DIER |= TIM_DIER_UIE;

  // Clear any pending interrupt flags
  TIM4->SR &= ~TIM_SR_UIF;

  // Button external interrupt will enable the timer
  TIM4->CR1 &= ~TIM_CR1_CEN;

  // Enable TIM4 interrupt in NVIC
  NVIC_EnableIRQ(TIM4_IRQn);
}

// EXTI0 Interrupt Handler
void EXTI0_IRQHandler(void)
{
  if(EXTI->PR & EXTI_PR_PR0)
  {
    EXTI->IMR &= ~EXTI_IMR_MR0;
    EXTI->PR |= EXTI_PR_PR0;

    TIM4->CNT = 0;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 |= TIM_CR1_CEN;
  }
}

void TIM4_IRQHandler(void)
{
  if(TIM4->SR & TIM_SR_UIF)
  {

    TIM4->CR1 &= ~TIM_CR1_CEN;

    if(!(GPIOA->IDR & GPIO_IDR_IDR0))
    {
      Button_NextMode();
    }
    else
    {
      // Do Nothing
    }

    EXTI->IMR |= EXTI_IMR_MR0;
    TIM4->SR &= ~TIM_SR_UIF;
  }
}

