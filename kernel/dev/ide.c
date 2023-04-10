#include "ide.h"

uint8_t k_IDE_ReadStatus(struct k_dev_ide_disk_t *disk)
{
    return disk->funcs.ReadReg8(disk, K_IDE_CMD_REG_STATUS);
}

uint8_t k_IDE_ReadError(struct k_dev_ide_disk_t *disk)
{
    return disk->funcs.ReadReg8(disk, K_IDE_CMD_REG_ERROR);
}

void k_IDE_ExecCmd(struct k_dev_ide_disk_t *disk, uint32_t cmd)
{
    while(k_IDE_ReadStatus(disk) & K_IDE_STATUS_FLAG_BSY);
    disk->funcs.WriteReg8(disk, K_IDE_CMD_REG_CMD, cmd);
}

uint32_t k_IDE_Read(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd)
{
    // struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)disk->data;
    struct k_dev_ide_disk_t *ide_disk = (struct k_dev_ide_disk_t *)disk;
    struct k_dev_dsk_ide_cmd_t *ide_cmd = (struct k_dev_dsk_ide_cmd_t *)cmd;
    uint32_t block_size = ide_disk->disk.block_size;
    uint32_t first_block = cmd->address / block_size;
    uint32_t last_block = ((ide_cmd->cmd.address + ide_cmd->cmd.size + block_size - 1) & (~(block_size - 1))) / block_size;
    uint32_t block_count = last_block - first_block;

    if(block_count)
    {
        /* keep only the byte offset into the sector. This will be used for handling not 
        sector/word aligned start addresses, and also as a total bytes copied counter */
        ide_cmd->skip_count = ide_cmd->cmd.address & 0x1ff;
        // cmd_state->cur_cmd = cmd;
        ide_cmd->cmd.address = 0;
        ide_disk->cmd = ide_cmd;
        // disk->data = ide_cmd;

        // k_sys_TerminalPrintf("k_PIIX3_IDE_Read: read %d sectors\n", block_count);

        while(block_count > 0)
        {
            uint32_t read_block_count = block_count;

            if(read_block_count > 256)
            {
                read_block_count = 256;
            }

            ide_cmd->transfer_condition = 0;
            ide_cmd->sector_count = read_block_count;

            while(!(k_IDE_ReadStatus(ide_disk) & K_IDE_STATUS_FLAG_DRDY));

            // k_cpu_OutB((first_block & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_NUMBER);
            // k_cpu_OutB(((first_block >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_LOW);
            // k_cpu_OutB(((first_block >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_HI);
            // k_cpu_OutB(((first_block >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DRV_HEAD);
            // k_cpu_OutB(read_block_count, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_COUNT);

            ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_SEC_NUMBER, first_block & 0xff);
            ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_CYL_LOW, (first_block >> 8) & 0xff);
            ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_CYL_HI, (first_block >> 16) & 0xff);
            ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_DRV_HEAD, ((first_block >> 24) & 0x0f) | 0xe0);
            ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_SEC_COUNT, read_block_count);

            k_IDE_ExecCmd(ide_disk, K_IDE_CMD_READ_SEC_NR);
            k_proc_WaitCondition(&ide_cmd->transfer_condition);
            block_count -= read_block_count;
            first_block += read_block_count;
        }
    }
    else
    {
        k_rt_SignalCondition(&ide_cmd->cmd.condition);
    }
    
    return 0;
}

// void k_IDE_ReadCmd(struct k_dev_device_t *device, struct k_dsk_cmd_t *cmd)
// {
    
// }

// void k_IDE_WriteCmd(struct k_dev_device_t *device, struct k_dsk_cmd_t *cmd)
// {
    
// }

// void k_IDE_IdentifyCmd(struct k_dev_device_t *device, struct k_dsk_cmd_t *cmd)
// {
    
// }

void k_IDE_InterruptHandler(struct k_dev_ide_disk_t *disk)
{
    (void)disk;
}