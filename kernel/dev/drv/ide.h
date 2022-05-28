#ifndef K_IDE_H
#define K_IDE_H

#include <stdint.h>
#include "../../dsk/defs.h"

enum K_IDE_CMD_REGS
{
    K_IDE_CMD_REG_DATA          = 0x00,
    K_IDE_CMD_REG_ERROR         = 0x01,
    K_IDE_CMD_REG_FEAT          = 0x01,
    K_IDE_CMD_REG_SEC_COUNT     = 0x02,
    K_IDE_CMD_REG_SEC_NUMBER    = 0x03,
    K_IDE_CMD_REG_CYL_LOW       = 0x04,
    K_IDE_CMD_REG_CYL_HI        = 0x05,
    K_IDE_CMD_REG_DRV_HEAD      = 0x06,
    K_IDE_CMD_REG_STATUS        = 0x07,
    K_IDE_CMD_REG_CMD           = 0x07
};

enum K_IDE_CMDS
{
    K_IDE_CMD_ACK_MEDIA_CHANGE  = 0xdb,
    K_IDE_CMD_POST_BOOT         = 0xdc,
    K_IDE_CMD_PRE_BOOT          = 0xdd,
    K_IDE_CMD_CHK_PWR_MODE      = 0xe598,
    K_IDE_CMD_DOOR_LOCK         = 0xde,
    K_IDE_CMD_DOOR_UNLOCK       = 0xdf,
    K_IDE_CMD_EXEC_DIAG         = 0x90,
    K_IDE_CMD_FORMAT_TRACK      = 0x50,
    K_IDE_CMD_ID_DRIVE          = 0xec,
    K_IDE_CMD_IDLE              = 0xe397,
    K_IDE_CMD_IDLE_IMM          = 0xe195,
    K_IDE_CMD_INIT_PARAMS       = 0x91,
    K_IDE_CMD_NOP               = 0x00,
    K_IDE_CMD_READ_BUFFER       = 0xe4,
    K_IDE_CMD_READ_DMA_R        = 0xc8,
    K_IDE_CMD_READ_DMA_NR       = 0xc9,
    K_IDE_CMD_READ_LONG_R       = 0x22,
    K_IDE_CMD_READ_LONG_NR      = 0x23,
    K_IDE_CMD_READ_MULT         = 0xc4,
    K_IDE_CMD_READ_SEC_R        = 0x20,
    K_IDE_CMD_READ_SEC_NR       = 0x21,
    K_IDE_CMD_VRFY_SEC_R        = 0x40,
    K_IDE_CMD_VRFY_SEC_NR       = 0x41,
    K_IDE_CMD_RECALIBRATE       = 0x10,
    K_IDE_CMD_SEEK              = 0x70,
};

enum K_IDE_STATUS_FLAGS
{
    K_IDE_STATUS_FLAG_ERROR     = 1,
    K_IDE_STATUS_FLAG_INDEX     = 1 << 1,
    K_IDE_STATUS_FLAG_CORR      = 1 << 2,
    K_IDE_STATUS_FLAG_DREQ      = 1 << 3,
    K_IDE_STATUS_FLAG_SEEK_OK   = 1 << 4,
    K_IDE_STATUS_FLAG_DRDY      = 1 << 6,
    K_IDE_STATUS_FLAG_BSY       = 1 << 7
};

enum K_IDE_ERROR_FLAGS
{
    K_IDE_ERROR_FLAG_AMNF   = 1,
    K_IDE_ERROR_FLAG_TKONF  = 1 << 1,
    K_IDE_ERROR_FLAG_ABRT   = 1 << 2,
};

struct k_ide_info_t
{
    uint16_t general_info;
    uint16_t cylinder_count;
    uint16_t reserved0;
    uint16_t head_count;
    uint16_t bytes_per_track;
    uint16_t bytes_per_sector;
    uint16_t vendor_unique0[3];
    uint16_t serial[10];
    uint16_t buffer_type;
    uint16_t buffer_size;
    uint16_t ecc_byte_count;
    uint16_t firmare_revision[4];
    uint16_t model_number[20];
    uint16_t vendor_unique1;
    uint16_t dword_io_capable;
    uint16_t capabilites;
    uint16_t reserved1;
    uint16_t pio_tx_time_mode;
    uint16_t dma_tx_time_moed;
    uint16_t valid_extra;
    uint16_t cur_cylinder_count;
    uint16_t cur_head_count;
    uint16_t cur_bytes_per_track;
    uint16_t cur_capacity[2];
    uint16_t ignore0;
    uint16_t lba_sector_count[2];
    uint16_t ignore3;
    uint16_t ignore4;
    uint16_t reserved2[64];
    uint16_t vendor_unique2[32];
    uint16_t reserved[97];
};

struct k_ide_device_t
{
    uint16_t    (*read_reg)(uint8_t reg);
    void        (*write_reg)(uint8_t reg, uint8_t value);
};

struct k_ide_cmd_state_t
{
    struct k_dsk_cmd_t *    cur_cmd;
    uint16_t                skip_count;
    uint16_t                align;
};

uint8_t k_IDE_ReadStatus(struct k_ide_device_t *device);

void k_IDE_ExecCmd(struct k_ide_device_t *device, uint8_t cmd);

void k_IDE_ReadCmd(struct k_dsk_cmd_t *cmd);

void k_IDE_WriteCmd(struct k_dsk_cmd_t *cmd);

void k_IDE_IdentifyCmd(struct k_dsk_cmd_t *cmd);

void k_IDE_InterruptHandler();

#endif