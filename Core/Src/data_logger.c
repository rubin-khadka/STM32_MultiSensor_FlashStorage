/*
 * data_logger.c
 *
 *  Created on: Feb 28, 2026
 *      Author: Rubin Khadka
 */

#include "data_logger.h"
#include "w25q64.h"
#include "mpu6050.h"
#include "ds18b20.h"
#include "timer2.h"
#include "uart.h"
#include "utils.h"

#define FLASH_PAGE_SIZE     256
#define ENTRY_SIZE          32
#define ENTRIES_PER_PAGE    8
#define START_PAGE          1
#define MAX_PAGES           100

static uint32_t entry_count = 0;
static uint8_t initialized = 0;

static void print_fixed_width(float value, uint8_t width, uint8_t decimals)
{
  char buffer[12];
  format_float(value, buffer, decimals, 0);

  // Calculate current length
  uint8_t len = 0;
  while(buffer[len])
    len++;

  // Print spaces for alignment
  for(uint8_t i = len; i < width; i++)
  {
    USART1_SendChar(' ');
  }

  // Print the number
  USART1_SendString(buffer);
}

// Initialize - count how many entries already exist
void DataLogger_Init(void)
{
  uint8_t buffer[ENTRY_SIZE];
  uint32_t count = 0;

  for(uint32_t page = START_PAGE; page < START_PAGE + MAX_PAGES; page++)
  {
    for(uint16_t offset = 0; offset < FLASH_PAGE_SIZE; offset += ENTRY_SIZE)
    {
      // Read one entry
      W25Q64_Read(page, offset, ENTRY_SIZE, buffer);

      // Check if empty (all 0xFF)
      uint8_t is_empty = 1;
      for(uint8_t i = 0; i < ENTRY_SIZE; i++)
      {
        if(buffer[i] != 0xFF)
        {
          is_empty = 0;
          break;
        }
      }

      if(is_empty)
      {
        page = START_PAGE + MAX_PAGES; // Break outer loop
        break;
      }
      count++;
    }
  }

  entry_count = count;
  initialized = 1;

  // Use your existing USART1_SendNumber function
  USART1_SendString("Found ");
  USART1_SendNumber(entry_count);
  USART1_SendString(" entries\r\n");
}

// Save current sensor data
uint8_t DataLogger_SaveEntry(void)
{
  if(!initialized)
    return LOGGER_UNINIT;

  uint8_t buffer[ENTRY_SIZE];
  uint8_t *ptr = buffer;

  // Calculate write position
  uint32_t total_bytes = entry_count * ENTRY_SIZE;
  uint32_t page = START_PAGE + (total_bytes / FLASH_PAGE_SIZE);
  uint16_t offset = total_bytes % FLASH_PAGE_SIZE;

  // Pack data into buffer (32 bytes total)

  // DS18B20 temperature (float = 4 bytes)
  uint32_t temp_val = *(uint32_t*) &ds18b20_data.temperature;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // MPU6050 temperature (float = 4 bytes)
  temp_val = *(uint32_t*) &mpu6050_scaled.temp;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Accelerometer X (float)
  temp_val = *(uint32_t*) &mpu6050_scaled.accel_x;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Accelerometer Y
  temp_val = *(uint32_t*) &mpu6050_scaled.accel_y;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Accelerometer Z
  temp_val = *(uint32_t*) &mpu6050_scaled.accel_z;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Gyro X
  temp_val = *(uint32_t*) &mpu6050_scaled.gyro_x;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Gyro Y
  temp_val = *(uint32_t*) &mpu6050_scaled.gyro_y;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Gyro Z
  temp_val = *(uint32_t*) &mpu6050_scaled.gyro_z;
  *ptr++ = (temp_val >> 0) & 0xFF;
  *ptr++ = (temp_val >> 8) & 0xFF;
  *ptr++ = (temp_val >> 16) & 0xFF;
  *ptr++ = (temp_val >> 24) & 0xFF;

  // Write to flash
  W25Q64_WritePage(page, offset, ENTRY_SIZE, buffer);

  entry_count++;

  USART1_SendString("\r\nSaved #");
  USART1_SendNumber(entry_count);
  USART1_SendString(" at page ");
  USART1_SendNumber(page);
  USART1_SendString(" offset ");
  USART1_SendNumber(offset);
  USART1_SendString("\r\n");

  return LOGGER_OK;
}

// Read all entries and display nicely
uint32_t DataLogger_ReadAll(void)
{
  if(!initialized || entry_count == 0)
  {
    USART1_SendString("No entries found\r\n");
    return 0;
  }

  uint8_t buffer[ENTRY_SIZE];
  uint32_t entries_read = 0;

  // Print table header
  USART1_SendString("\r\n");
  USART1_SendString(
      "+-----+----------+----------+----------+----------+----------+----------+----------+----------+\r\n");
  USART1_SendString(
      "|  #  | DS18B20  | MPU6050  |  Accel X |  Accel Y |  Accel Z |  Gyro X  |  Gyro Y  |  Gyro Z  |\r\n");
  USART1_SendString(
      "+-----+----------+----------+----------+----------+----------+----------+----------+----------+\r\n");

  for(uint32_t i = 0; i < entry_count; i++)
  {
    uint32_t page = START_PAGE + ((i * ENTRY_SIZE) / FLASH_PAGE_SIZE);
    uint16_t offset = (i * ENTRY_SIZE) % FLASH_PAGE_SIZE;

    W25Q64_Read(page, offset, ENTRY_SIZE, buffer);

    float ds18b20_temp = *(float*) &buffer[0];
    float mpu6050_temp = *(float*) &buffer[4];
    float accel_x = *(float*) &buffer[8];
    float accel_y = *(float*) &buffer[12];
    float accel_z = *(float*) &buffer[16];
    float gyro_x = *(float*) &buffer[20];
    float gyro_y = *(float*) &buffer[24];
    float gyro_z = *(float*) &buffer[28];

    // Print row
    USART1_SendString("| ");

    // Entry number (width 3)
    if(i + 1 < 10)
      USART1_SendString("  ");
    else if(i + 1 < 100)
      USART1_SendString(" ");
    USART1_SendNumber(i + 1);
    USART1_SendString(" | ");

    // Temperature (width 8)
    print_fixed_width(ds18b20_temp, 8, 2);
    USART1_SendString(" | ");

    // MPU6050 Temperature (width 8)
    print_fixed_width(mpu6050_temp, 8, 2);
    USART1_SendString(" | ");

    // Accel values (width 8 each)
    print_fixed_width(accel_x, 8, 2);
    USART1_SendString(" | ");
    print_fixed_width(accel_y, 8, 2);
    USART1_SendString(" | ");
    print_fixed_width(accel_z, 8, 2);
    USART1_SendString(" | ");

    // Gyro values (width 8 each)
    print_fixed_width(gyro_x, 8, 2);
    USART1_SendString(" | ");
    print_fixed_width(gyro_y, 8, 2);
    USART1_SendString(" | ");
    print_fixed_width(gyro_z, 8, 2);
    USART1_SendString(" | ");

    USART1_SendString("\r\n");
    entries_read++;
  }

  // Print table footer
  USART1_SendString(
      "+-----+----------+----------+----------+----------+----------+----------+----------+----------+\r\n");
  USART1_SendString("Total: ");
  USART1_SendNumber(entries_read);
  USART1_SendString(" entries\r\n");

  return entries_read;
}

uint32_t DataLogger_GetEntryCount(void)
{
  return entry_count;
}
