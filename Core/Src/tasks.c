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
