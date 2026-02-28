/*
 * tasks.c
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "timer2.h"
#include "uart.h"
#include "button.h"
#include "utils.h"
#include "mpu6050.h"
#include "i2c2.h"
#include "lcd.h"
#include "ds18b20.h"

static char uart_buf[32];

static char uart_buf[32];

// Structure for feedback display
typedef struct
{
  char line1[16];
  char line2[16];
  uint32_t end_time;      // System time when feedback should end
  uint8_t active;         // Feedback currently active
} Feedback_t;

static Feedback_t feedback = {0};

// Show a message on LCD for specified duration
void Feedback_Show(const char *line1, const char *line2, uint16_t duration_ms)
{
  // Copy line1
  int i = 0;
  while(line1[i] && i < 15)
  {
    feedback.line1[i] = line1[i];
    i++;
  }
  feedback.line1[i] = '\0';

  // Copy line2
  i = 0;
  while(line2[i] && i < 15)
  {
    feedback.line2[i] = line2[i];
    i++;
  }
  feedback.line2[i] = '\0';

  // Calculate end time
  feedback.end_time = TIMER2_GetMillis() + duration_ms;
  feedback.active = 1;

  // Show immediately on LCD
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_SendString(feedback.line1);
  LCD_SetCursor(1, 0);
  LCD_SendString(feedback.line2);

  // Optional: Also send to UART for debugging
  USART1_SendString("Feedback: ");
  USART1_SendString(line1);
  USART1_SendString(" - ");
  USART1_SendString(line2);
  USART1_SendString("\r\n");
}

// Check if feedback time has expired
void Task_Feedback_Update(void)
{
  if(!feedback.active)
    return;

  uint32_t now = TIMER2_GetMillis();

  // Check if we've reached/passed the end time
  if(now >= feedback.end_time)
  {
    feedback.active = 0;
  }
}

// Force clear feedback immediately
void Feedback_Clear(void)
{
  feedback.active = 0;
}

// Check if feedback is currently active
uint8_t Feedback_IsActive(void)
{
  return feedback.active;
}

// Task to update UART output
void Task_UART_Output(void)
{
  DisplayMode_t mode = Button_GetMode();

  switch(mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      format_reading(ds18b20_data.temperature, mpu6050_scaled.temp, uart_buf);
      break;

    case DISPLAY_MODE_ACCEL:
      format_accel_scaled(uart_buf, mpu6050_scaled.accel_x, mpu6050_scaled.accel_y, mpu6050_scaled.accel_z, 2);
      break;

    case DISPLAY_MODE_GYRO:
      format_gyro_scaled(uart_buf, mpu6050_scaled.gyro_x, mpu6050_scaled.gyro_y, mpu6050_scaled.gyro_z, 2);
      break;

    default:
      return;
  }

  USART1_SendString(uart_buf);
}

// Task to read DS18b20 sensor
void Task_DS18B20_Read(void)
{
  // Read the temperature
  float temp = DS18B20_ReadTemperature();

  if(temp > -100.0f)
  {
    ds18b20_data.temperature = temp;
    ds18b20_data.valid = 1;
  }
  else
  {
    ds18b20_data.valid = 0;
  }

  // Immediately start next conversion
  DS18B20_StartConversion();
}

// Task to read MPU6050 sensor
void Task_MPU6050_Read(void)
{
  if(MPU6050_ReadAll() == I2C_OK)
  {
    MPU6050_ScaleAll();
  }
}

// Task to update LCD display
void Task_LCD_Update(void)
{
  if(feedback.active) return;
  DisplayMode_t mode = Button_GetMode();

  switch(mode)
  {
    case DISPLAY_MODE_TEMP_HUM:
      LCD_DisplayReading(ds18b20_data.temperature, mpu6050_scaled.temp);
      break;

    case DISPLAY_MODE_ACCEL:
      LCD_DisplayAccelScaled(mpu6050_scaled.accel_x, mpu6050_scaled.accel_y, mpu6050_scaled.accel_z);
      break;

    case DISPLAY_MODE_GYRO:
      LCD_DisplayGyroScaled(mpu6050_scaled.gyro_x, mpu6050_scaled.gyro_y, mpu6050_scaled.gyro_z);
      break;

    default:  // Handles DISPLAY_MODE_COUNT and any invalid values
      break;
  }
}
