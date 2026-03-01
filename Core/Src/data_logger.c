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

#define SECTORS_FOR_DATA    100      // Use 2047 sectors
#define ENTRIES_PER_SECTOR  128       // 128 entries per sector
#define MAX_ENTRIES         (SECTORS_FOR_DATA * ENTRIES_PER_SECTOR)

static uint32_t entry_count = 0;
static uint8_t initialized = 0;

// Erase state machine
typedef enum
{
  ERASE_IDLE,
  ERASE_WAITING,     // Waiting for 400ms to pass
  ERASE_VERIFY       // Checking if erase completed
} EraseState_t;

static struct
{
  uint32_t write_index;             // Current write position
  uint32_t total_saved;             // Total entries ever saved
  uint32_t pending_sector;          // Sector being erased
  EraseState_t erase_state;         // Current erase state
  uint32_t erase_start_time;        // When erase was started
  uint8_t erase_retry_count;        // For safety
} logger;

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
  uint32_t last_valid_pos = 0;
  uint8_t found_entries = 0;

  logger.write_index = 0;
  logger.total_saved = 0;
  logger.erase_state = ERASE_IDLE;

  USART1_SendString("Scanning for existing data...\r\n");

  // Scan all sectors in circular order
  for(uint32_t sector = 0; sector < 1 + SECTORS_FOR_DATA; sector++)
  {
    for(uint32_t entry_in_sector = 0; entry_in_sector < ENTRIES_PER_SECTOR; entry_in_sector++)
    {
      uint32_t entry_pos = (sector - 1) * ENTRIES_PER_SECTOR + entry_in_sector;
      uint32_t page = (sector * 16) + (entry_in_sector / 8);
      uint16_t offset = (entry_in_sector % 8) * 32;

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

      if(!is_empty) // Found valid entries
      {
        count++;
        last_valid_pos = entry_pos;
        found_entries = 1;
      }
    }
  }

  if(found_entries)
  {
    // Set write_index to next position after last valid entry
    logger.write_index = last_valid_pos + 1;
    logger.total_saved = count;
    entry_count = count;

    USART1_SendString("Found ");
    USART1_SendNumber(count);
    USART1_SendString(" entries. Next write at position ");
    USART1_SendNumber(logger.write_index);
    USART1_SendString("\r\n");
  }
  else
  {
    // No existing data
    logger.write_index = 0;
    logger.total_saved = 0;
    entry_count = 0;

    USART1_SendString("No existing data found\r\n");
  }

  initialized = 1;
}

// Save current sensor data
uint8_t DataLogger_SaveEntry(void)
{
  if(!initialized)
    return LOGGER_UNINIT;

  // If erase is in progress, don't save yet
  if(logger.erase_state != ERASE_IDLE)
  {
    return LOGGER_BUSY;
  }

  // Calculate circular position
  uint32_t entry_pos = logger.write_index % MAX_ENTRIES;
  uint32_t sector = 1 + (entry_pos / ENTRIES_PER_SECTOR);

  // Check if we need to erase this sector
  if(entry_pos % ENTRIES_PER_SECTOR == 0)
  {
    // This is the first entry in a new sector - start erase
    USART1_SendString("Starting erase for sector ");
    USART1_SendNumber(sector);
    USART1_SendString("\r\n");

    // Start non-blocking erase
    W25Q64_EraseSector(sector);

    // Record erase state
    logger.pending_sector = sector;
    logger.erase_start_time = TIMER2_GetMillis();
    logger.erase_state = ERASE_WAITING;

    logger.write_index++;

    return LOGGER_BUSY;
  }

  uint8_t buffer[ENTRY_SIZE];
  uint8_t *ptr = buffer;

  // Calculate page and offset within the sector
  uint32_t page = (sector * 16) + ((entry_pos % ENTRIES_PER_SECTOR) / 8);
  uint16_t offset = ((entry_pos % ENTRIES_PER_SECTOR) % 8) * 32;

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

  // Update indices
  logger.write_index++;
  logger.total_saved++;
  entry_count = logger.total_saved;  // Keep entry_count for compatibility

  USART1_SendString("\r\nSaved #");
  USART1_SendNumber(logger.total_saved);
  USART1_SendString(" at sector ");
  USART1_SendNumber(sector);
  USART1_SendString(" page ");
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

  // Determine how many entries to read
  uint32_t entries_to_read = (logger.total_saved < MAX_ENTRIES) ? logger.total_saved : MAX_ENTRIES;

  // Calculate start position
  uint32_t start_pos = (logger.write_index >= entries_to_read) ? (logger.write_index - entries_to_read) : 0;

  // Print table header
  USART1_SendString("\r\n");
  USART1_SendString(
      "+-----+----------+----------+----------+----------+----------+----------+----------+----------+\r\n");
  USART1_SendString(
      "|  #  | DS18B20  | MPU6050  |  Accel X |  Accel Y |  Accel Z |  Gyro X  |  Gyro Y  |  Gyro Z  |\r\n");
  USART1_SendString(
      "+-----+----------+----------+----------+----------+----------+----------+----------+----------+\r\n");

  for(uint32_t i = 0; i < entries_to_read; i++)
  {
    uint32_t entry_pos = (start_pos + i) % MAX_ENTRIES;
    uint32_t sector = 1 + (entry_pos / ENTRIES_PER_SECTOR);
    uint32_t page = (sector * 16) + ((entry_pos % ENTRIES_PER_SECTOR) / 8);
    uint16_t offset = ((entry_pos % ENTRIES_PER_SECTOR) % 8) * 32;

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

void Task_DataLogger(void)
{
  uint32_t now = TIMER2_GetMillis();

  switch(logger.erase_state)
  {
    case ERASE_WAITING:
      // Wait 400ms for erase to complete
      if((now - logger.erase_start_time) >= 400)
      {
        // Time's up, now verify
        logger.erase_state = ERASE_VERIFY;
        logger.erase_retry_count = 0;
      }
      break;

    case ERASE_VERIFY:
      // Check status register (quick SPI operation)
      if(!(W25Q64_ReadStatus() & 0x01))  // BUSY bit cleared
      {
        USART1_SendString("Sector ");
        USART1_SendNumber(logger.pending_sector);
        USART1_SendString(" erase complete\r\n");
        logger.erase_state = ERASE_IDLE;
      }
      else
      {
        logger.erase_retry_count++;
        if(logger.erase_retry_count > 10)  // 10 tries = 100ms
        {
          // Something wrong, give up
          USART1_SendString("Erase timeout!\r\n");
          logger.erase_state = ERASE_IDLE;
        }
        // Otherwise check again next 10ms tick
      }
      break;

    case ERASE_IDLE:

    default:
      break;
  }
}
