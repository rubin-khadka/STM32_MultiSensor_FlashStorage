#include "main.h"
#include "w25q64.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;
#define W25Q_SPI hspi1

#define numBLOCK 32  // number of total blocks for 16Mb flash

void W25Q_Delay(uint32_t time)
{
  HAL_Delay(time);
}

void csLOW (void)
{
  HAL_GPIO_WritePin (GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

void csHIGH (void)
{
  HAL_GPIO_WritePin (GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void SPI_Write (uint8_t *data, uint8_t len)
{
  HAL_SPI_Transmit(&W25Q_SPI, data, len, 2000);
}

void SPI_Read (uint8_t *data, uint8_t len)
{
  HAL_SPI_Receive(&W25Q_SPI, data, len, 5000);
}

/**************************************************************************************************/

void W25Q_Reset (void)
{
  uint8_t tData[2];
  tData[0] = 0x66;  // enable Reset
  tData[1] = 0x99;  // Reset
  csLOW();
  SPI_Write(tData, 2);
  csHIGH();
  W25Q_Delay(100);
}

uint32_t W25Q_ReadID (void)
{
  uint8_t tData = 0x9F;
  uint8_t rData[3];
  csLOW();
  SPI_Write(&tData, 1);

  // First byte is dummy/status - discard it
  SPI_Read(rData, 3);
  csHIGH();

  // rData[0] is dummy (0xFF)
  // rData[1] is Manufacturer (0xEF)
  // rData[2] is Device Type (0x40)

  return ((rData[1] << 16) | (rData[2] << 8) | 0x17);  // Add capacity manually
}
