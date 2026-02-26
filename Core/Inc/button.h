/*
 * button.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#ifndef BUTTON_H_
#define BUTTON_H_

// Display modes
typedef enum
{
  DISPLAY_MODE_TEMP_HUM = 0,  // DHT11 temperature and humidity
  DISPLAY_MODE_ACCEL,         // MPU6050 accelerometer
  DISPLAY_MODE_GYRO,          // MPU6050 gyroscope
  DISPLAY_MODE_COUNT
} DisplayMode_t;

void Button_Init(void);
DisplayMode_t Button_GetMode(void);
void Button_NextMode(void);
void EXTI0_IRQHandler(void);
void TIMER4_Init(void);
void TIM4_IRQHandler(void);

#endif /* BUTTON_H_ */
