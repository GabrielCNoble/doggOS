#ifndef K_PIIX3_IDE_H
#define K_PIIX3_IDE_H

#include <stdint.h>
#include "../../drv/ide.h"
#include "../../../dsk/defs.h"

#define K_PIIX3_IDE_CH0_PCI_IDETIM_REG 0x40

#define K_PIIX3_PRIMARY_IDE_CMD_BLOCK 0x1f0
#define K_PIIX3_IDE_DATA_REG          0x00
#define K_PIIX3_IDE_ERROR_REG         0x01
#define K_PIIX3_IDE_SEC_COUNT_REG     0x02
#define K_PIIX3_IDE_SEC_NUMBER_REG    0x03
#define K_PIIX3_IDE_CYL_LOW_REG       0x04
#define K_PIIX3_IDE_CYL_HIGH_REG      0x05
#define K_PIIX3_IDE_DRIVE_REG         0x06
#define K_PIIX3_IDE_HEAD_REG          0x06
#define K_PIIX3_IDE_CMD_REG           0x07
#define K_PIIX3_IDE_STATUS_REG        0x07

uint32_t k_PIIX3_IDE_Init(uint8_t bus_index, uint8_t device_index);

uint8_t k_PIIX3_IDE_ReadStatus();

uint8_t K_PIIX3_IDE_ReadError();

void k_PIIX3_IDE_ExecCmd(uint8_t cmd);

// void k_PIIX3_IDE_Read(uint32_t lba, uint32_t size);

uint32_t k_PIIX3_IDE_Read(struct k_dsk_cmd_t *cmd);

void k_PIIX3_IDE_Handler();

#endif