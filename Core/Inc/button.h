/*
 * button.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#ifndef BUTTON_H_
#define BUTTON_H_

// Display modes structure
typedef enum
{
  DISPLAY_MODE_TEMP_HUM = 0,  // DHT11 temperature and humidity
  DISPLAY_MODE_ACCEL,         // MPU6050 accelerometer
  DISPLAY_MODE_GYRO,          // MPU6050 gyroscope
  DISPLAY_MODE_COUNT
} DisplayMode_t;

extern volatile uint8_t g_button2_pressed;
extern volatile uint8_t g_button3_pressed;

// Function Prototypes
void Button_Init(void);
void TIMER4_Init(void);

// Display swap mode functions
DisplayMode_t Button_GetMode(void);
void Button_NextMode(void);

#endif /* BUTTON_H_ */
