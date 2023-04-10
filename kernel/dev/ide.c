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
        ide_cmd->cmd.address = 0;
        ide_disk->cmd = ide_cmd;

        while(block_count > 0)
        {
            uint32_t read_block_count = block_count;

            if(read_block_count > K_DEV_IDE_MAX_SECTORS_PER_CMD)
            {
                read_block_count = K_DEV_IDE_MAX_SECTORS_PER_CMD;
            }

            ide_cmd->transfer_condition = 0;
            ide_cmd->sector_count = read_block_count;

            while(!(k_IDE_ReadStatus(ide_disk) & K_IDE_STATUS_FLAG_DRDY));

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
    struct k_dev_ide_disk_t *ide_disk = (struct k_dev_ide_disk_t *)disk;
    struct k_dev_dsk_ide_cmd_t *ide_cmd = ide_disk->cmd;
    
    if(k_IDE_ReadStatus(ide_disk) & K_IDE_STATUS_FLAG_ERROR)
    {        
        k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)k_IDE_ReadError(ide_disk));
        k_rt_SignalCondition(&ide_cmd->cmd.condition);
    }
    else
    {        
        uint32_t available_bytes = ide_disk->disk.block_size;

        switch(ide_cmd->cmd.type)
        {
            case K_DEV_DSK_CMD_TYPE_WRITE:
                // if(cmd->address)
                // {
                //     /* address is not sector aligned, so we'll just throw out the data we don't need */
                //     uint32_t word_count = (cmd->address & 0x1ff) / sizeof(uint16_t);
                //     for(uint32_t index = 0; index < word_count; index++)
                //     {
                //         // k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //         k_cpu_OutW(0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //     }
                //     cmd->address -= word_count * sizeof(uint16_t);
                // 
                //     if(cmd->address & 0x1)
                //     {
                //         /* address is not word aligned, so we'll read a word here and discard half of it */
                //         // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //         uint16_t data = ((uint8_t *)cmd->buffer)[0];
                //         data <<= 8;
                //         k_cpu_OutW(data, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //         // ((uint8_t *)cmd->buffer)[0] = (uint8_t)(data >> 8);
                //         cmd->address--;
                //         cmd->size--;
                //     }
                // }
                // 
                // if(cmd->size > 1)
                // {
                //     /* bulk of the copy  */
                //     uint32_t copy_size = 512 / sizeof(uint16_t);
                //     uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
                // 
                //     if(copy_size > buffer_word_count)
                //     {
                //         copy_size = buffer_word_count;
                //     }
                // 
                //     copy_size /= sizeof(temp_data) / sizeof(temp_data[0]);
                //     for(uint32_t index = 0; index < copy_size; index++)
                //     {
                //         // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //         k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG, temp_data, sizeof(temp_data) / sizeof(temp_data[0]));
                //         k_rt_CopyBytes((uint8_t *)cmd->buffer + cmd->address, temp_data, sizeof(temp_data));
                //         cmd->address += sizeof(temp_data);
                //         cmd->size -= sizeof(temp_data);
                //     }
                // }
                // else if(cmd->size == 1)
                // {
                //     /* the size of the buffer is not a round number of words, so copy the last byte here */
                //     uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                //     ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
                //     cmd->size--;
                // }
            break;
            
            case K_DEV_DSK_CMD_TYPE_READ:
            case K_DEV_DSK_CMD_TYPE_IDENTIFY:
                // uint32_t bytes_left = cmd->size - cmd->address;
                if(ide_cmd->skip_count)
                {
                    /* copy is not sector aligned, so we'll just throw out the data we don't need */
                    uint32_t word_count = ide_cmd->skip_count / sizeof(uint16_t);
                    for(uint32_t index = 0; index < word_count; index++)
                    {
                        ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
                    }

                    available_bytes -= word_count * sizeof(uint16_t);
                    
                    
                    if(ide_cmd->skip_count & 0x1)
                    {
                        /* address is not word aligned, so we'll read a word here and discard the first half */
                        uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
                        /* address is pointing at the second byte of a word */
                        ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)(data >> 8);
                        ide_cmd->cmd.address++;
                        available_bytes -= 2;
                    }

                    ide_cmd->skip_count = 0;

                    if(!available_bytes)
                    {
                        /* we actually read a whole sector from the sector buffer, so get out */
                        break;
                    }
                }
                
                if(available_bytes > 1)
                {
                    /* bulk of the copy  */
                    uint32_t copy_size = available_bytes / sizeof(uint16_t);
                    uint32_t buffer_word_count = ide_cmd->cmd.size >> 1;
                    // uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
                    
                    if(copy_size > buffer_word_count)
                    {
                        copy_size = buffer_word_count;
                    }

                    if(ide_cmd->cmd.address & 1)
                    {
                        /* Well, shit, unaligned copy */
                        for(uint32_t word_index = 0; word_index < copy_size; word_index++)
                        {
                            uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
                            ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)data;
                            ide_cmd->cmd.address++;
                            ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)(data >> 8);
                            ide_cmd->cmd.address++;
                        }
                    }
                    else
                    {
                        /* Cool, fast(er) aligned copy */
                        uint16_t *buffer = (uint16_t *)((uint8_t *)ide_cmd->cmd.buffer + ide_cmd->cmd.address);
                        ide_disk->funcs.ReadReg16S(ide_disk, K_IDE_CMD_REG_DATA, copy_size, buffer);
                        ide_cmd->cmd.address += copy_size * sizeof(uint16_t);                    
                    }
                    
                    available_bytes -= copy_size * sizeof(uint16_t);
                }
                
                if(available_bytes == 1)
                {
                    /* the size of the buffer is not a round number of words, so copy the last byte here */
                    // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
                    ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)data;
                    ide_cmd->cmd.address++;
                }
                else
                {
                    /* apparently the IDE controller expects us to read a whole sector...? Not
                    doing so causes the kernel to hang on qemu. Works fine on bochs, though. The
                    AT attachment spec says the host reads a sector of data during pio transfers, 
                    but doesn't seem to assert it's absolutely required to read the whole sector
                    buffer. */
                    uint32_t word_count = available_bytes / sizeof(uint16_t);
                    for(uint32_t word_index = 0; word_index < word_count; word_index++)
                    {
                        ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
                    }
                }
            break;
        }

        ide_cmd->sector_count--;
        
        if(ide_cmd->cmd.address == ide_cmd->cmd.size)
        {
            /* let anything waiting for this command to know we're done */
            k_rt_SignalCondition(&ide_cmd->cmd.condition);
        }

        if(ide_cmd->sector_count == 0)
        {
            k_rt_SignalCondition(&ide_cmd->transfer_condition);
        }
    }
}