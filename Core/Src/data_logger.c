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
#include "dwt.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

// Flash memory layout definitions
#define FLASH_PAGE_SIZE     256     // Bytes per page
#define FLASH_SECTOR_SIZE   4096    // Bytes per sector (16 pages)

#define HEADER_PAGE         0       // Page 0 for header
#define DATA_START_PAGE     16      // Start storing data from page 16 (sector 1)
#define MAX_LOG_PAGES       100     // Use 100 pages for logs

#define LOGGER_MAGIC        0x4C4F4731  // "LOG1" in ASCII

// Header structure (stored at page 0)
typedef struct
{
  uint32_t magic;                 // Magic number to identify valid format
  uint32_t entry_count;           // Total number of entries saved
  uint32_t next_page;             // Next free page to write to
  uint32_t next_offset;           // Next offset within page
} LoggerHeader_t;

// Logger header
static LoggerHeader_t header;
static uint8_t initialized = 0;

// Initialize the logger
void DataLogger_Init(void)
{
  USART1_SendString("\r\n=== Initializing Data Logger ===\r\n");

  // Read header from flash (page 0, offset 0)
  W25Q64_Read(HEADER_PAGE, 0, sizeof(LoggerHeader_t), (uint8_t*) &header);

  // Check if header is valid
  if(header.magic != LOGGER_MAGIC)
  {
    USART1_SendString("First time use - creating new log\r\n");

    // Initialize new header
    header.magic = LOGGER_MAGIC;
    header.entry_count = 0;
    header.next_page = DATA_START_PAGE;       // First data page (16)
    header.next_offset = 0;                   // Start at beginning of page

    // Save header to flash
    W25Q64_WritePage(HEADER_PAGE, 0, sizeof(LoggerHeader_t), (uint8_t*) &header);
    DWT_Delay_ms(10);  // Small delay for flash to settle

    USART1_SendString("New log created\r\n");
  }
  else
  {
    USART1_SendString("Existing log found\r\n");
  }

  initialized = 1;

  // Print current status
  char buf[60];
  sprintf(
      buf,
      "Status: %lu entries, Next page: %lu, Offset: %lu\r\n",
      header.entry_count,
      header.next_page,
      header.next_offset);
  USART1_SendString(buf);
  USART1_SendString("=== Logger Ready ===\r\n");
}
