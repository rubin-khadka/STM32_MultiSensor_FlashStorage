/*
 * ds18b20.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "stm32f103xb.h"
#include "ds18b20.h"
#include "dwt.h"
#include "timer2.h"

// Pin Definitions
#define DS18B20_GPIO  GPIOB
#define DS18B20_PIN   0

// Pin Operations
#define DS18B20_HIGH()  (DS18B20_GPIO->BSRR = GPIO_BSRR_BS0)
#define DS18B20_LOW()   (DS18B20_GPIO->BRR = GPIO_BRR_BR0)
#define DS18B20_READ()  ((DS18B20_GPIO->IDR >> DS18B20_PIN) & 1)

// Commands
#define DS18B20_CMD_SKIP_ROM          0xCC
#define DS18B20_CMD_CONVERT_T         0x44
#define DS18B20_CMD_READ_SCRATCHPAD   0xBE

volatile DS18B20_Data_t ds18b20_data = {0, 0};

void DS18B20_Init(void)
{
  // Enable GPIOB Clock
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  // Set as output, High initially
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_0;   // General purpose output open drain
  DS18B20_GPIO->CRL |= GPIO_CRL_MODE0_0;  // 10 MHz
  DS18B20_HIGH(); // Pin high
}

int DS18B20_Reset(void)
{
  int presence = 0;
  uint32_t timeout;

  // Set as output, pull low for reset pulse
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_0;   // General purpose output open drain
  DS18B20_GPIO->CRL |= GPIO_CRL_MODE0_0;  // 10 MHz

  DS18B20_LOW();
  DWT_Delay_us(500);  // 500µs reset pulse (>480µs)

  // Set as input (release line), enable pull-up
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_1;  // Input with pull-up
  DS18B20_GPIO->ODR |= (1 << DS18B20_PIN);  // Pull-up enabled

  // Wait for sensor to respond with timeout
  timeout = 100;  // About 100µs timeout
  while(DS18B20_READ() && timeout--)  // Wait for line to go low
  {
    DWT_Delay_us(1);
  }

  if(!DS18B20_READ())  // Line is low (presence detected)
  {
    presence = 1;

    // Wait for presence pulse to end (max 240µs)
    timeout = 300;
    while(!DS18B20_READ() && timeout--)
    {
      DWT_Delay_us(1);
    }
  }

  return presence;
}

void DS18B20_WriteBit(int bit)
{
  // Set as output
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_0;   // General purpose output open drain
  DS18B20_GPIO->CRL |= GPIO_CRL_MODE0_0;  // 10 MHz

  DS18B20_LOW();  // Start time slot

  if(bit)
  {
    // Write 1: release within 15µs
    DWT_Delay_us(2);
    DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
    DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_1;  // Input with pull-up
    DS18B20_GPIO->ODR |= (1 << DS18B20_PIN);
    DWT_Delay_us(60);
  }
  else
  {
    // Write 0: hold low entire slot
    DWT_Delay_us(60);
    DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
    DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_1;  // Input with pull-up
    DS18B20_GPIO->ODR |= (1 << DS18B20_PIN);
    DWT_Delay_us(2);
  }
}

int DS18B20_ReadBit(void)
{
  int bit;

  // Set as output to start read slot
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_0;  // Output open-drain
  DS18B20_GPIO->CRL |= GPIO_CRL_MODE0_0;

  DS18B20_LOW();
  DWT_Delay_us(2);  // Hold low min 1µs

  DS18B20_HIGH();

  // Set as input, sensor takes over
  DS18B20_GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  DS18B20_GPIO->CRL |= GPIO_CRL_CNF0_1;  // Input with pull-up
  DS18B20_GPIO->ODR |= (1 << DS18B20_PIN);

  DWT_Delay_us(10);  // Sample within 15µs

  bit = DS18B20_READ();

  DWT_Delay_us(50);  // Complete the 60µs slot

  return bit;
}

void DS18B20_WriteByte(uint8_t data)
{
  for(int i = 0; i < 8; i++)
  {
    DS18B20_WriteBit(data & 0x01);
    data >>= 1;
  }
}

uint8_t DS18B20_ReadByte(void)
{
  uint8_t data = 0;

  for(int i = 0; i < 8; i++)
  {
    data >>= 1;
    if(DS18B20_ReadBit())
    {
      data |= 0x80;
    }
  }

  return data;
}

float DS18B20_ReadTemperature(void)
{
  uint8_t lsb, msb;
  int16_t raw;

  if(!DS18B20_Reset())
    return -999.0f;

  DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
  DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);

  lsb = DS18B20_ReadByte();
  msb = DS18B20_ReadByte();

  raw = (msb << 8) | lsb;
  return raw * 0.0625f;
}

void DS18B20_StartConversion(void)
{
  if(!DS18B20_Reset())
    return;

  DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
  DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);
}
