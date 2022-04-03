#ifndef K_IDE_H
#define K_IDE_H

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

#endif