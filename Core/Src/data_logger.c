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
#include <string.h>
#include <stdio.h>


#define FLASH_PAGE_SIZE     256
#define ENTRY_SIZE          32
#define ENTRIES_PER_PAGE    8
#define START_PAGE          1
#define MAX_PAGES           100

static uint32_t entry_count = 0;
static uint8_t initialized = 0;

// Initialize - count how many entries already exist
void DataLogger_Init(void)
{
  uint8_t buffer[ENTRY_SIZE];
  uint32_t count = 0;
  uint8_t empty_count = 0;

  USART1_SendString("\r\n=== Scanning for existing data ===\r\n");

  // Scan through pages to find where data ends
  for(uint32_t page = START_PAGE; page < START_PAGE + MAX_PAGES; page++)
  {
    for(uint16_t offset = 0; offset < FLASH_PAGE_SIZE; offset += ENTRY_SIZE)
    {
      // Read one entry
      W25Q64_Read(page, offset, ENTRY_SIZE, buffer);

      // Check if this entry is empty (all 0xFF)
      uint8_t is_empty = 1;
      for(int i = 0; i < ENTRY_SIZE; i++)
      {
        if(buffer[i] != 0xFF)
        {
          is_empty = 0;
          break;
        }
      }

      if(is_empty)
      {
        // Found first empty spot, stop scanning
        USART1_SendString("Found empty spot. Scan complete.\r\n");
        page = START_PAGE + MAX_PAGES; // Break outer loop
        break;
      }
      else
      {
        count++; // This is a valid entry
      }
    }
  }

  entry_count = count;
  initialized = 1;

  char msg[50];
  sprintf(msg, "Found %lu existing entries\r\n", entry_count);
  USART1_SendString(msg);
  USART1_SendString("=== Logger Ready ===\r\n");
}

// Save current sensor data
uint8_t DataLogger_SaveEntry(void)
{
  if(!initialized)
    return LOGGER_UNINIT;

  LogEntry_t entry;
  uint8_t buffer[ENTRY_SIZE];

  // Calculate where to write next
  uint32_t total_bytes = entry_count * ENTRY_SIZE;
  uint32_t page = START_PAGE + (total_bytes / FLASH_PAGE_SIZE);
  uint16_t offset = total_bytes % FLASH_PAGE_SIZE;

  // Prepare entry
  entry.ds18b20_temp = ds18b20_data.temperature;
  entry.accel_x = mpu6050_scaled.accel_x;
  entry.accel_y = mpu6050_scaled.accel_y;
  entry.accel_z = mpu6050_scaled.accel_z;
  entry.gyro_x = mpu6050_scaled.gyro_x;
  entry.gyro_y = mpu6050_scaled.gyro_y;
  entry.gyro_z = mpu6050_scaled.gyro_z;
  entry.timestamp = TIMER2_GetMillis();

  memcpy(buffer, &entry, ENTRY_SIZE);

  USART1_SendString("\r\nWriting to page: ");
  USART1_SendNumber(page);
  USART1_SendString(", offset: ");
  USART1_SendNumber(offset);
  USART1_SendString("\r\n");

  // Write to flash
  W25Q64_WritePage(page, offset, ENTRY_SIZE, buffer);

  entry_count++;

  char msg[30];
  sprintf(msg, "Saved entry #%lu\r\n", entry_count);
  USART1_SendString(msg);

  return LOGGER_OK;
}

// Read ALL entries
uint32_t DataLogger_ReadAll(void)
{
  if(!initialized || entry_count == 0)
  {
    USART1_SendString("No entries found\r\n");
    return 0;
  }

  LogEntry_t entry;
  uint8_t buffer[ENTRY_SIZE];
  uint32_t entries_read = 0;

  USART1_SendString("\r\n====================================================================\r\n");
  USART1_SendString("NUM | DS18B20 | ACCEL X | ACCEL Y | ACCEL Z | GYRO X | GYRO Y | GYRO Z | TIME(ms)\r\n");
  USART1_SendString("====================================================================\r\n");

  // Read all entries sequentially
  for(uint32_t i = 0; i < entry_count; i++)
  {
    uint32_t total_bytes = i * ENTRY_SIZE;
    uint32_t page = START_PAGE + (total_bytes / FLASH_PAGE_SIZE);
    uint16_t offset = total_bytes % FLASH_PAGE_SIZE;

    W25Q64_Read(page, offset, ENTRY_SIZE, buffer);
    memcpy(&entry, buffer, ENTRY_SIZE);

    char line[120];
    sprintf(
        line,
        " %3lu |  %5.2f  |  %6.1f | %6.1f | %6.1f | %5.1f | %5.1f | %5.1f |  %lu\r\n",
        i + 1,
        entry.ds18b20_temp,
        entry.accel_x,
        entry.accel_y,
        entry.accel_z,
        entry.gyro_x,
        entry.gyro_y,
        entry.gyro_z,
        entry.timestamp);
    USART1_SendString(line);

    entries_read++;
  }

  USART1_SendString("====================================================================\r\n");
  char summary[50];
  sprintf(summary, "Total: %lu entries\r\n", entries_read);
  USART1_SendString(summary);

  return entries_read;
}

uint32_t DataLogger_GetEntryCount(void)
{
  return entry_count;
}
