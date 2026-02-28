/*
 * w25q64.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef W25Q64_H_
#define W25Q64_H_

// W25Q64 Commands
#define W25Q64_CMD_RESET            0x99
#define W25Q64_CMD_RESET_ENABLE     0x66
#define W25Q64_CMD_READ_ID          0x9F
#define W25Q64_CMD_READ_STATUS      0x05
#define W25Q64_CMD_WRITE_ENABLE     0x06
#define W25Q64_CMD_WRITE_DISABLE    0x04
#define W25Q64_CMD_READ_DATA        0x03
#define W25Q64_CMD_PAGE_PROGRAM     0x02

#define W25Q64_CMD_WRITE_STATUS     0x01
#define W25Q64_CMD_FAST_READ        0x0B

#define W25Q64_CMD_SECTOR_ERASE     0x20
#define W25Q64_CMD_BLOCK_ERASE_32K  0x52
#define W25Q64_CMD_BLOCK_ERASE_64K  0xD8
#define W25Q64_CMD_CHIP_ERASE       0xC7
#define W25Q64_CMD_READ_ID          0x9F
#define W25Q64_CMD_RESET            0x99
#define W25Q64_CMD_RESET_ENABLE     0x66

// Function Prototypes
void W25Q64_Reset(void);
void W25Q64_Init(void);
uint8_t W25Q64_ReadStatus(void);
void W25Q64_WriteEnable(void);
void W25Q64_WriteDisable(void);
void W25Q64_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
void W25Q64_WritePage(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);

#endif /* W25Q64_H_ */
