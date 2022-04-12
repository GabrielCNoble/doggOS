#include "ide.h"

uint8_t k_IDE_ReadStatus(struct k_ide_device_t *device)
{
    return (uint8_t)device->read_reg(K_IDE_CMD_REG_STATUS);
}

void k_IDE_ExecCmd(struct k_ide_device_t *device, uint8_t cmd)
{
    while(!(k_IDE_ReadStatus(device) & K_IDE_STATUS_FLAG_DRDY));
    device->write_reg(K_IDE_CMD_REG_CMD, cmd);
}

void k_IDE_ReadCmd(struct k_dsk_cmd_t *cmd)
{
    
}

void k_IDE_WriteCmd(struct k_dsk_cmd_t *cmd)
{
    
}

void k_IDE_IdentifyCmd(struct k_dsk_cmd_t *cmd)
{
    
}