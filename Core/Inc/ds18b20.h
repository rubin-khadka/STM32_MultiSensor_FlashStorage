/*
 * ds18b20.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "stdint.h"

// Structure to hold temperature file
typedef struct
{
  float temperature;
  uint8_t valid;
} DS18B20_Data_t;

extern volatile DS18B20_Data_t ds18b20_data;

// Function Prototypes
void DS18B20_Init(void);
int DS18B20_Reset(void);
void DS18B20_WriteByte(uint8_t data);
uint8_t DS18B20_ReadByte(void);
float DS18B20_ReadTemperature(void);
void DS18B20_StartConversion(void);

#endif /* DS18B20_H_ */
