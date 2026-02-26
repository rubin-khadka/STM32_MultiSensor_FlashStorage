/*
 * utils.c
 *
 *  Created on: Feb 20, 2026
 *      Author: Rubin Khadka
 */

#include "utils.h"

// Convert 16 bit integer to string
void itoa_16(int16_t value, char *buffer)
{
  char *ptr = buffer;

  // Handle negative numbers
  if(value < 0)
  {
    *ptr++ = '-';
    value = -value;
  }

  // Extract digits in reverse order
  char temp[6];
  uint8_t i = 0;
  do
  {
    temp[i++] = (value % 10) + '0';
    value /= 10;
  }
  while(value > 0);

  // Reverse digits into buffer
  while(i-- > 0)
  {
    *ptr++ = temp[i];
  }
  *ptr = '\0';
}

// Convert 8 bit integer to string
void itoa_8(uint8_t value, char *buffer)
{
  char *ptr = buffer;
  char temp[3];
  uint8_t i = 0;

  // Special case for zero
  if(value == 0)
  {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  // Extract digits
  do
  {
    temp[i++] = (value % 10) + '0';
    value /= 10;
  }
  while(value > 0);

  // Reverse digits
  while(i-- > 0)
  {
    *ptr++ = temp[i];
  }
  *ptr = '\0';
}

// Format integer value for UART output
void format_value(uint8_t integer, uint8_t decimal, char *buffer, char unit)
{
  char *ptr = buffer;

  // Integer part
  if(integer >= 10)
  {
    *ptr++ = '0' + (integer / 10);
  }
  *ptr++ = '0' + (integer % 10);

  // Decimal point and tenths
  *ptr++ = '.';
  *ptr++ = '0' + decimal;

  // Unit
  *ptr++ = unit;
  *ptr = '\0';
}

void format_reading(float temp_ds18b20, float temp_mpu6050, char *buffer)
{
  char *ptr = buffer;
  char temp_buffer[16];

  *ptr++ = 'T';
  *ptr++ = 'E';
  *ptr++ = 'M';
  *ptr++ = 'P';
  *ptr++ = ' ';
  *ptr++ = 'm';
  *ptr++ = 'p';
  *ptr++ = 'u';
  *ptr++ = ':';
  *ptr++ = ' ';

  format_float(temp_mpu6050, temp_buffer, 2, 'C');

  for(char *s = temp_buffer; *s; s++)
  {
    *ptr++ = *s;
  }

  *ptr++ = ' ';
  *ptr++ = 'T';
  *ptr++ = 'E';
  *ptr++ = 'M';
  *ptr++ = 'P';
  *ptr++ = ' ';
  *ptr++ = 'd';
  *ptr++ = 's';
  *ptr++ = '1';
  *ptr++ = '8';
  *ptr++ = ':';
  *ptr++ = ' ';

  format_float(temp_ds18b20, temp_buffer, 2, 'C');

  for(char *s = temp_buffer; *s; s++)
  {
    *ptr++ = *s;
  }

  // Add newline
  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';
}

void format_accel(char *buffer, int16_t ax, int16_t ay, int16_t az)
{
  char *ptr = buffer;
  char num[8];

  // "AX:123 AY:456 AZ:789"

  // AX
  *ptr++ = 'A';
  *ptr++ = 'X';
  *ptr++ = ':';
  itoa_16(ax, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = ' ';

  // AY
  *ptr++ = 'A';
  *ptr++ = 'Y';
  *ptr++ = ':';
  itoa_16(ay, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = ' ';

  // AZ
  *ptr++ = 'A';
  *ptr++ = 'Z';
  *ptr++ = ':';
  itoa_16(az, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';
}

void format_gyro(char *buffer, int16_t gx, int16_t gy, int16_t gz)
{
  char *ptr = buffer;
  char num[8];

  // "GX:123 GY:456 GZ:789"

  // GX
  *ptr++ = 'G';
  *ptr++ = 'X';
  *ptr++ = ':';
  itoa_16(gx, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = ' ';

  // GY
  *ptr++ = 'G';
  *ptr++ = 'Y';
  *ptr++ = ':';
  itoa_16(gy, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = ' ';

  // GZ
  *ptr++ = 'G';
  *ptr++ = 'Z';
  *ptr++ = ':';
  itoa_16(gz, num);
  for(char *s = num; *s; s++)
    *ptr++ = *s;

  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';
}

/* --------------------- Functions for Scaled Values--------------------------- */

// Convert float to string with specified decimal places
void ftoa(float value, char *buffer, uint8_t decimal_places)
{
  char *ptr = buffer;

  // Handle negative numbers
  if(value < 0)
  {
    *ptr++ = '-';
    value = -value;
  }

  // Extract integer part
  int32_t int_part = (int32_t) value;

  // Extract fractional part
  float fractional = value - int_part;

  // Handle integer part
  char temp[16];
  uint8_t i = 0;

  // Special case for zero
  if(int_part == 0)
  {
    temp[i++] = '0';
  }
  else
  {
    // Extract integer digits in reverse
    while(int_part > 0)
    {
      temp[i++] = (int_part % 10) + '0';
      int_part /= 10;
    }
  }

  // Reverse integer digits into buffer
  while(i-- > 0)
  {
    *ptr++ = temp[i];
  }

  // Add decimal point
  *ptr++ = '.';

  // Handle fractional part
  for(uint8_t j = 0; j < decimal_places; j++)
  {
    fractional *= 10;
    uint8_t digit = (uint8_t) fractional;
    *ptr++ = digit + '0';
    fractional -= digit;
  }

  *ptr = '\0';
}

// Format float with unit
void format_float(float value, char *buffer, uint8_t decimal_places, char unit)
{
  char *ptr = buffer;
  char num[16];

  ftoa(value, num, decimal_places);

  // Copy the number
  for(char *s = num; *s; s++)
  {
    *ptr++ = *s;
  }

  // Add unit
  *ptr++ = unit;
  *ptr = '\0';
}

// Format scaled accelerometer data
void format_accel_scaled(char *buffer, float ax, float ay, float az, uint8_t decimal_places)
{
  char *ptr = buffer;
  char num[16];

  // "AX:1.23g AY:4.56g AZ:7.89g"

  // AX
  *ptr++ = 'A';
  *ptr++ = 'X';
  *ptr++ = ':';
  ftoa(ax, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'g';
  *ptr++ = ' ';

  // AY
  *ptr++ = 'A';
  *ptr++ = 'Y';
  *ptr++ = ':';
  ftoa(ay, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'g';
  *ptr++ = ' ';

  // AZ
  *ptr++ = 'A';
  *ptr++ = 'Z';
  *ptr++ = ':';
  ftoa(az, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'g';

  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';
}

// Format scaled gyroscope data
void format_gyro_scaled(char *buffer, float gx, float gy, float gz, uint8_t decimal_places)
{
  char *ptr = buffer;
  char num[16];

  // "GX:1.23dps GY:4.56dps GZ:7.89dps"

  // GX
  *ptr++ = 'G';
  *ptr++ = 'X';
  *ptr++ = ':';
  ftoa(gx, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'd';
  *ptr++ = 'p';
  *ptr++ = 's';
  *ptr++ = ' ';

  // GY
  *ptr++ = 'G';
  *ptr++ = 'Y';
  *ptr++ = ':';
  ftoa(gy, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'd';
  *ptr++ = 'p';
  *ptr++ = 's';
  *ptr++ = ' ';

  // GZ
  *ptr++ = 'G';
  *ptr++ = 'Z';
  *ptr++ = ':';
  ftoa(gz, num, decimal_places);
  for(char *s = num; *s; s++)
    *ptr++ = *s;
  *ptr++ = 'd';
  *ptr++ = 'p';
  *ptr++ = 's';

  *ptr++ = '\r';
  *ptr++ = '\n';
  *ptr = '\0';
}
