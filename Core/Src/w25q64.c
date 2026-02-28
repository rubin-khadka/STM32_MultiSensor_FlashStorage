#include "main.h"
#include "W25q64.h"
#include "spi1.h"
#include "dwt.h"
#include "lcd.h"
#include "uart.h"

// W25Q64 Commands
#define W25Q64_CMD_RESET            0x99
#define W25Q64_CMD_RESET_ENABLE     0x66
#define W25Q64_CMD_READ_ID          0x9F
#define W25Q64_CMD_READ_STATUS      0x05
#define W25Q64_CMD_WRITE_ENABLE     0x06
#define W25Q64_CMD_WRITE_DISABLE    0x04
#define W25Q64_CMD_READ_DATA        0x03
#define W25Q64_CMD_FAST_READ        0x0B
#define W25Q64_CMD_PAGE_PROGRAM     0x02
#define W25Q64_CMD_SECTOR_ERASE     0x20

#define W25Q64_CMD_WRITE_STATUS     0x01
#define W25Q64_CMD_BLOCK_ERASE_32K  0x52
#define W25Q64_CMD_BLOCK_ERASE_64K  0xD8
#define W25Q64_CMD_CHIP_ERASE       0xC7

void W25Q64_Reset(void)
{
  SPI1_CS_LOW();

  SPI1_Transfer(W25Q64_CMD_RESET_ENABLE);   // Enable Reset
  SPI1_Transfer(W25Q64_CMD_RESET);          // Reset command
  while(SPI1->SR & SPI_SR_BSY);
  SPI1_CS_HIGH();

  DWT_Delay_ms(100);  // 100 MILLISECONDS
}

// Initialization and Identification
void W25Q64_Init(void)
{
  uint8_t manufacturer_id = 0;
  uint8_t memory_type = 0;
  uint8_t capacity = 0;

  // Small delay for power-up
  DWT_Delay_ms(100);

  // Read full JEDEC ID
  SPI1_CS_LOW();
  SPI1_Transfer(W25Q64_CMD_READ_ID);  // 0x9F

  manufacturer_id = SPI1_Transfer(0xFF);
  memory_type = SPI1_Transfer(0xFF);
  capacity = SPI1_Transfer(0xFF);

  SPI1_CS_HIGH();

  // Check manufacturer ID (should be 0xEF for Winbond)
  if(manufacturer_id == 0xEF)
  {
    LCD_Clear();
    LCD_SendString("W25Q64 Flash");
    LCD_SetCursor(1, 0);

    char msg[16];
    msg[0] = "0123456789ABCDEF"[manufacturer_id >> 4];
    msg[1] = "0123456789ABCDEF"[manufacturer_id & 0x0F];
    msg[2] = ' ';
    msg[3] = "0123456789ABCDEF"[memory_type >> 4];
    msg[4] = "0123456789ABCDEF"[memory_type & 0x0F];
    msg[5] = ' ';
    msg[6] = "0123456789ABCDEF"[capacity >> 4];
    msg[7] = "0123456789ABCDEF"[capacity & 0x0F];
    msg[8] = 0;
    LCD_SendString("ID: ");
    LCD_SendString(msg);

    // UART output
    USART1_SendString("W25Q64 Found! JEDEC ID: ");
    USART1_SendHex(manufacturer_id);
    USART1_SendString(" ");
    USART1_SendHex(memory_type);
    USART1_SendString(" ");
    USART1_SendHex(capacity);
    USART1_SendString("\r\n");

    // Check if it's really W25Q64 (memory type 0x40, capacity 0x17)
    if(memory_type == 0x40 && capacity == 0x17)
    {
      USART1_SendString("W25Q64 Detected (64Mbit)\r\n");
    }
  }
  else
  {
    LCD_Clear();
    LCD_SendString("W25Q64 ERROR!");
    LCD_SetCursor(1, 0);
    LCD_SendString("ID: ");

    char hex[3];
    hex[0] = "0123456789ABCDEF"[manufacturer_id >> 4];
    hex[1] = "0123456789ABCDEF"[manufacturer_id & 0x0F];
    hex[2] = 0;
    LCD_SendString(hex);

    USART1_SendString("W25Q64 Error! Manufacturer ID: 0x");
    USART1_SendHex(manufacturer_id);
    USART1_SendString("\r\n");
  }
}

uint8_t W25Q64_ReadStatus(void)
{
  uint8_t status;

  SPI1_CS_LOW();
  // Send read status register command (0x05)
  SPI1_Transfer(W25Q64_CMD_READ_STATUS);  // 0x05
  // Read status byte
  status = SPI1_Transfer(0xFF);
  SPI1_CS_HIGH();

  return status;
}

void W25Q64_WriteEnable(void)
{
  uint8_t status;
  uint32_t timeout = 1000;

  SPI1_CS_LOW();
  // Send write enable command (0x06)
  SPI1_Transfer(W25Q64_CMD_WRITE_ENABLE);  // 0x06
  SPI1_CS_HIGH();

  USART1_SendString("Checking for status !\r\n");
  do
  {
    status = W25Q64_ReadStatus();
    timeout--;
    if(timeout == 0)
    {
      // Write enable failed
      USART1_SendString("Write Enable Timeout!\r\n");
      break;
    }
  }
  while(!(status & 0x02));  // Wait for WEL bit (bit 1)
  USART1_SendString("Write Enabled !\r\n");
}

void W25Q64_WriteDisable(void)
{
  SPI1_CS_LOW();
  SPI1_Transfer(W25Q64_CMD_WRITE_DISABLE);  // 0x04
  SPI1_CS_HIGH();
  DWT_Delay_ms(5);
}

void W25Q64_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
{
  uint32_t memAddr = (startPage * 256) + offset;

  SPI1_CS_LOW();                    // Pull CS low
  // Send read command (0x03)
  SPI1_Transfer(W25Q64_CMD_READ_DATA);

  // Send 24-bit address (MSB first)
  SPI1_Transfer((memAddr >> 16) & 0xFF);    // Address bits 23-16
  SPI1_Transfer((memAddr >> 8) & 0xFF);     // Address bits 15-8
  SPI1_Transfer(memAddr & 0xFF);            // Address bits 7-0

  // Read data bytes
  for(uint32_t i = 0; i < size; i++)
  {
    rData[i] = SPI1_Transfer(0xFF);     // Send dummy byte, read data
  }

  SPI1_CS_HIGH();                   // Pull CS high
  DWT_Delay_us(5);                  // Small delay after CS high
}

void W25Q64_FastRead(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
{
  uint32_t memAddr = (startPage * 256) + offset;

  SPI1_CS_LOW();

  // Send fast read command (0x0B)
  SPI1_Transfer(W25Q64_CMD_FAST_READ);

  // Send 24-bit address (MSB first)
  SPI1_Transfer((memAddr >> 16) & 0xFF);    // Address bits 23-16
  SPI1_Transfer((memAddr >> 8) & 0xFF);     // Address bits 15-8
  SPI1_Transfer(memAddr & 0xFF);            // Address bits 7-0

  // Dummy byte (required for fast read at higher speeds)
  SPI1_Transfer(0x00);

  // Small delay before reading data
  DWT_Delay_us(1);

  // Read data bytes
  for(uint32_t i = 0; i < size; i++)
  {
    rData[i] = SPI1_Transfer(0xFF);       // Send dummy, read data
  }

  SPI1_CS_HIGH();
  DWT_Delay_us(5);
}

uint32_t bytestowrite(uint32_t size, uint16_t offset)
{
  if((size + offset) < 256)
  {
    return size;           // Can write all in one page
  }
  else
  {
    return 256 - offset;    // Fill up to page boundary
  }
}

uint8_t W25Q64_EraseSector(uint32_t sector_num)
{
  uint32_t memAddr = sector_num * 4096;  // Each sector = 16 pages * 256 bytes = 4096 bytes

  USART1_SendString("Erasing Sector ");
  USART1_SendNumber(sector_num);
  USART1_SendString("...\r\n");

  // Enable write and verify
  W25Q64_WriteEnable();

  // Check if write enable succeeded
  uint8_t status = W25Q64_ReadStatus();
  if(!(status & 0x02))
  {
    USART1_SendString("ERROR: Write Enable Failed for Erase!\r\n");
    return W25Q64_ERROR;
  }

  // Send sector erase command (0x20 for 24-bit addressing)
  SPI1_CS_LOW();

  SPI1_Transfer(W25Q64_CMD_SECTOR_ERASE);                    // Sector Erase command
  SPI1_Transfer((memAddr >> 16) & 0xFF);   // Address bits 23-16
  SPI1_Transfer((memAddr >> 8) & 0xFF);    // Address bits 15-8
  SPI1_Transfer(memAddr & 0xFF);            // Address bits 7-0

  // Wait for SPI to finish
  while(SPI1->SR & SPI_SR_BSY);

  SPI1_CS_HIGH();

  return W25Q64_OK;
}

void W25Q64_WritePage(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
{
  uint32_t startPage = page;
  uint32_t endPage = startPage + ((size + offset - 1) / 256);
  uint32_t numPages = endPage - startPage + 1;

  uint32_t dataPos = 0;
  uint32_t remaining = size;
  uint16_t currentOffset = offset;
  uint32_t currentPage = page;

  // Write the data page by page
  while(remaining > 0)
  {
    uint16_t bytesThisPage = (remaining + currentOffset < 256) ? remaining : 256 - currentOffset;

    uint32_t memAddr = (currentPage * 256) + currentOffset;

    W25Q64_WriteEnable();

    // Page program
    SPI1_CS_LOW();

    SPI1_Transfer(W25Q64_CMD_PAGE_PROGRAM);   // Page Program command, 0x02
    SPI1_Transfer((memAddr >> 16) & 0xFF);    // Address MSB
    SPI1_Transfer((memAddr >> 8) & 0xFF);     // Address middle
    SPI1_Transfer(memAddr & 0xFF);            // Address LSB

    // Send data for this page
    for(uint16_t i = 0; i < bytesThisPage; i++)
    {
      SPI1_Transfer(data[dataPos + i]);
    }

    while(SPI1->SR & SPI_SR_BSY);
    SPI1_CS_HIGH();

    W25Q64_WriteDisable();

    // Update for next page
    dataPos += bytesThisPage;
    remaining -= bytesThisPage;
    currentPage++;
    currentOffset = 0;
  }
}
