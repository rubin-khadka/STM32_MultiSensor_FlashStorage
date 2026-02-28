/*
 * button.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "button.h"
#include "timer2.h"

#define COOLDOWN_PERIOD_MS 1000  // 1 second cooldown

// Current display mode
static volatile DisplayMode_t current_mode = DISPLAY_MODE_TEMP_HUM;

// Button states for debouncing
static volatile uint8_t button1_pressed = 0;  // PA0 - Mode switch
static volatile uint8_t button2_pressed = 0;  // PA1 - Save
static volatile uint8_t button3_pressed = 0;  // PA2 - Read

volatile uint8_t g_button2_pressed = 0;
volatile uint8_t g_button3_pressed = 0;

// Button cooldown timers
static volatile uint32_t button2_cooldown_until = 0;
static volatile uint32_t button3_cooldown_until = 0;

void Button_Init(void)
{
  // Enable Clocks
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

  // GPIO Configuration for button
  // A0 for display mode switch
  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
  GPIOA->CRL |= GPIO_CRL_CNF0_1;  // Input mode push-pull
  GPIOA->ODR |= GPIO_ODR_ODR0;    // GPIO pull-up

  // A1 for save and dump
  GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
  GPIOA->CRL |= GPIO_CRL_CNF1_1;  // Input mode push-pull
  GPIOA->ODR |= GPIO_ODR_ODR1;    // GPIO pull-up

  // A2 for delete
  GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
  GPIOA->CRL |= GPIO_CRL_CNF2_1;  // Input mode push-pull
  GPIOA->ODR |= GPIO_ODR_ODR2;    // GPIO pull-up

  // Connect PA0 to External Interrupt 0
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;

  // Connect PA1 to External Interrupt 1
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA;

  // Connect PA2 to External Interrupt 2
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI2;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA;

  // Disable interrupt while configuring
  EXTI->IMR &= ~(EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2);

  // Configure trigger edge (Low)
  EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR2;
  EXTI->RTSR &= ~(EXTI_RTSR_TR0 | EXTI_RTSR_TR1 | EXTI_RTSR_TR2);

  // Clear any pending interrupt
  EXTI->PR |= EXTI_PR_PR0 | EXTI_PR_PR1 | EXTI_PR_PR2;

  // Enable interrupt
  EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2;

  // Enable in NVIC
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_EnableIRQ(EXTI2_IRQn);

  // Initialize cooldown timers
  button2_cooldown_until = 0;
  button3_cooldown_until = 0;
}

// Check if button is in cooldown
static uint8_t Button_IsInCooldown(uint32_t cooldown_until)
{
  uint32_t current_time = TIMER2_GetMillis();

  if(cooldown_until == 0)
    return 0;  // Not in cooldown

  if(current_time >= cooldown_until)
    return 0;  // Cooldown expired

  return 1;  // Still in cooldown
}

// EXTI0 Interrupt Handler
void EXTI0_IRQHandler(void)
{
  if(EXTI->PR & EXTI_PR_PR0)
  {
    EXTI->IMR &= ~EXTI_IMR_MR0;
    EXTI->PR |= EXTI_PR_PR0;

    button1_pressed = 1;

    TIM4->CNT = 0;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 |= TIM_CR1_CEN;
  }
}

// EXTI1 Button 2
void EXTI1_IRQHandler(void)
{
  if(EXTI->PR & EXTI_PR_PR1)
  {
    // Check if button is in cooldown
    if(Button_IsInCooldown(button2_cooldown_until))
    {
      // Button is in cooldown - ignore this press
      EXTI->PR |= EXTI_PR_PR1;  // Clear pending bit
      return;  // Exit without processing
    }

    EXTI->IMR &= ~EXTI_IMR_MR1;
    EXTI->PR |= EXTI_PR_PR1;

    button2_pressed = 1;

    TIM4->CNT = 0;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 |= TIM_CR1_CEN;
  }
}

// EXTI2 Button 3
void EXTI2_IRQHandler(void)
{
  if(EXTI->PR & EXTI_PR_PR2)
  {
    // Check if button is in cooldown
    if(Button_IsInCooldown(button3_cooldown_until))
    {
      // Button is in cooldown - ignore this press
      EXTI->PR |= EXTI_PR_PR2;  // Clear pending bit
      return;  // Exit without processing
    }

    EXTI->IMR &= ~EXTI_IMR_MR2;
    EXTI->PR |= EXTI_PR_PR2;

    button3_pressed = 1;

    TIM4->CNT = 0;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 |= TIM_CR1_CEN;
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

void TIM4_IRQHandler(void)
{
  if(TIM4->SR & TIM_SR_UIF)
  {
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->CR1 &= ~TIM_CR1_CEN;

    // Button 1
    if(button1_pressed)
    {
      if(!(GPIOA->IDR & GPIO_IDR_IDR0))
      {
        Button_NextMode();
        button1_pressed = 0;
      }
      EXTI->IMR |= EXTI_IMR_MR0;
    }

    // Button 2
    if(button2_pressed)
    {
      if(!(GPIOA->IDR & GPIO_IDR_IDR1))
      {
        // Set cooldown for 1 second from now
        button2_cooldown_until = TIMER2_GetMillis() + COOLDOWN_PERIOD_MS;

        // Trigger the button action
        g_button2_pressed = 1;
        button2_pressed = 0;
      }
      EXTI->IMR |= EXTI_IMR_MR1;
    }

    // Button 3
    if(button3_pressed)
    {
      if(!(GPIOA->IDR & GPIO_IDR_IDR2))
      {
        // Set cooldown for 1 second from now
        button3_cooldown_until = TIMER2_GetMillis() + COOLDOWN_PERIOD_MS;

        // Trigger the button action
        g_button3_pressed = 1;
        button3_pressed = 0;
      }
      EXTI->IMR |= EXTI_IMR_MR2;
    }
  }
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

  /*  // Debug message
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
   }*/
}
