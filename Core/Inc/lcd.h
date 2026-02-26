/*
 * lcd.h
 *
 *  Created on: Feb 21, 2026
 *      Author: Rubin Khadka
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f103xb.h"
#include "i2c2.h"

// LCD I2C address (8 bit address 0x4E)
#define LCD_ADDR 0x27

// Function Prototypes
// Basic functions
void LCD_Init(void);
void LCD_SendCmd(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *str);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_DisplayError(void);

// Functions to display raw values
void LCD_DisplayGyro(int16_t gx, int16_t gy, int16_t gz);
void LCD_DisplayAccel(int16_t ax, int16_t ay, int16_t az);

// Functions to display scaled values
void LCD_DisplayReading(float temp_ds18b20, float temp_mpu6050);
void LCD_DisplayFloat(float value, uint8_t decimal_places);
void LCD_DisplayAccelScaled(float ax, float ay, float az);
void LCD_DisplayGyroScaled(float gx, float gy, float gz);

#endif /* LCD_H_ */
