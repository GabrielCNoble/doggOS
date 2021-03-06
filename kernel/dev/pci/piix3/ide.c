#include "ide.h"
#include "isa.h"
#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../int/irq.h"
#include "../../../io.h"
#include "../../../dsk/dsk.h"
#include "../../../rt/mem.h"
#include "../../../sys/term.h"

// struct k_io_stream_t *k_PIIX3_IDE_stream;
extern void *k_PIIX3_IDE_Handler_a;

// uint32_t K_PIIX3_IDE_total_transferred;
struct k_dsk_cmd_t *k_PIIX3_IDE_current_cmd;
struct k_dsk_disk_t *k_PIIX3_IDE_disk;
struct k_ide_device_t k_PIIX3_IDE_device;

uint32_t k_PIIX3_IDE_Init(uint8_t bus_index, uint8_t device_index, uint8_t function_index)
{
    // uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX, 0);
    (void)bus_index;
    (void)device_index;
    
    // k_PIIX3_IDE_device.read_reg = 
    
    struct k_dsk_cmd_t id_cmd = {};
    struct k_ide_info_t ide_info = {};
    id_cmd.type = K_DSK_CMD_TYPE_IDENTIFY;
    id_cmd.buffer = &ide_info;
    id_cmd.size = sizeof(struct k_ide_info_t);
    id_cmd.address = 0;
    k_int_SetInterruptHandler(K_PIIX3_IDE_IRQ_VECTOR, (uintptr_t)&k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);
    
    
    k_cpu_EnableInterrupts();
    k_PIIX3_IDE_Identify(&id_cmd);
    while(!id_cmd.condition);
    k_cpu_DisableInterrupts();
    
    uint32_t lba_sector_count = (((uint32_t)ide_info.lba_sector_count[0]) << 16) | ((uint32_t)ide_info.lba_sector_count[1]);
    k_PIIX3_IDE_disk = k_dsk_CreateDisk(ide_info.bytes_per_sector, lba_sector_count, 0, k_PIIX3_IDE_Read, NULL);
    
    return 0;
}

uint16_t k_PIIX3_IDE_ReadReg(uint8_t reg)
{
    
}

void k_PIIX3_IDE_WriteReg(uint8_t reg, uint16_t value)
{
    
}

uint8_t k_PIIX3_IDE_ReadStatus()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_STATUS);
}

uint8_t K_PIIX3_IDE_ReadError()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_ERROR);
}

void k_PIIX3_IDE_ExecCmd(uint8_t cmd)
{
    while(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_BSY);
    k_cpu_OutB(cmd, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CMD);
}

// void k_PIIX3_IDE_Delay()
// {
//     for(uint32_t x = 0; x < 0xffff; x++);
// }

// uint32_t read_done = 1;

// void k_PIIX3_IDE_Read(uint32_t lba, uint32_t size)
// {
//     while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
// 
//     k_cpu_OutB((lba & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_NUMBER_REG);
//     k_cpu_OutB(((lba >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_LOW_REG);
//     k_cpu_OutB(((lba >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_HIGH_REG);
//     k_cpu_OutB(((lba >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_HEAD_REG);
//     k_cpu_OutB(size, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_COUNT_REG);
//     k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
// }

uint32_t k_PIIX3_IDE_Read(struct k_dsk_cmd_t *cmd)
{
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    
    k_PIIX3_IDE_current_cmd = cmd;
    uint32_t lba = cmd->address >> 9;
    uint32_t size = cmd->size >> 9;
    
    if(cmd->size & 0x1ff)
    {
        size++;
    }
    
    /* keep only the byte offset into the sector. This will be used for handling not 
    sector/word aligned start addresses, and also as a total bytes copied counter */
    cmd->address &= 0x1ff;
    
    k_cpu_OutB((lba & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_NUMBER);
    k_cpu_OutB(((lba >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_LOW);
    k_cpu_OutB(((lba >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_HI);
    k_cpu_OutB(((lba >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DRV_HEAD);
    k_cpu_OutB(size, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_COUNT);
    k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
    
    return 0;
}

uint32_t k_PIIX3_IDE_Identify(struct k_dsk_cmd_t *cmd)
{
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    k_PIIX3_IDE_current_cmd = cmd;
    cmd->address = 0;
    k_PIIX3_IDE_ExecCmd(K_IDE_CMD_ID_DRIVE);
}

void k_PIIX3_IDE_Handler()
{
    struct k_dsk_cmd_t *cmd = k_PIIX3_IDE_current_cmd;
    // k_sys_TerminalPrintf("fuck\n");
    // uint16_t temp_data[32];
    if(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_ERROR)
    {
        // uint8_t error = K_PIIX3_IDE_ReadError();
        
        k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)K_PIIX3_IDE_ReadError());
        k_rt_SignalCondition(&cmd->condition);
    }
    else
    {        
        switch(cmd->type)
        {
            case K_DSK_CMD_TYPE_WRITE:
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
            
            case K_DSK_CMD_TYPE_READ:
            case K_DSK_CMD_TYPE_IDENTIFY:
                if(cmd->address)
                {
                    /* address is not sector aligned, so we'll just throw out the data we don't need */
                    uint32_t word_count = (cmd->address & 0x1ff) / sizeof(uint16_t);
                    for(uint32_t index = 0; index < word_count; index++)
                    {
                        k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    }
                    cmd->address -= word_count * sizeof(uint16_t);
                    
                    if(cmd->address & 0x1)
                    {
                        /* address is not word aligned, so we'll read a word here and discard half of it */
                        uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                        ((uint8_t *)cmd->buffer)[0] = (uint8_t)(data >> 8);
                        cmd->address--;
                        cmd->size--;
                    }
                }
                
                if(cmd->size > 1)
                {
                    /* bulk of the copy  */
                    uint32_t copy_size = 512 / sizeof(uint16_t);
                    uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
                    
                    if(copy_size > buffer_word_count)
                    {
                        copy_size = buffer_word_count;
                    }
                    // k_sys_TerminalPrintf("%d %d %d\n", cmd->size, copy_size, buffer_word_count);
                    
                    // copy_size /= sizeof(temp_data) / sizeof(temp_data[0]);
                    // for(uint32_t index = 0; index < copy_size; index++)
                    // {
                    //     k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG, temp_data, sizeof(temp_data) / sizeof(temp_data[0]));
                    //     k_rt_CopyBytes((uint8_t *)cmd->buffer + cmd->address, temp_data, sizeof(temp_data));
                    //     cmd->address += sizeof(temp_data);
                    //     cmd->size -= sizeof(temp_data);
                    // }
                    
                    uint16_t *buffer = (uint16_t *)((uint8_t *)cmd->buffer + cmd->address);
                    k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA, buffer, copy_size);
                    cmd->address += copy_size * sizeof(uint16_t);
                    cmd->size -= copy_size * sizeof(uint16_t);
                    
                    // k_sys_TerminalPrintf("%d %d %d\n", cmd->size, copy_size, buffer_word_count);
                }
                
                if(cmd->size == 1)
                {
                    /* the size of the buffer is not a round number of words, so copy the last byte here */
                    uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
                    cmd->size--;
                }
                
                if(cmd->address & 0x1ff)
                {
                    /* apparently the IDE controller expects us to read a whole sector...? Not
                    doing so causes the kernel to hang on qemu. Works fine on bochs, though. The
                    AT attachment spec says the host reads a sector of data during pio transfers, 
                    but doesn't seem to assert it's absolutely required to read the whole sector
                    buffer. */
                    uint32_t word_count = (512 - (cmd->address & 0x1ff)) / sizeof(uint16_t);
                    for(uint32_t word_index = 0; word_index < word_count; word_index++)
                    {
                        k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    }
                }
            break;
        }
        
        // k_sys_TerminalPrintf("blah\n");
        
        // k_sys_TerminalPrintf("read size: %d\n", cmd->size);
        
        if(!cmd->size)
        {
            /* let anything waiting for this command to know we're done */
            k_rt_SignalCondition(&cmd->condition);
            // k_sys_TerminalPrintf("signal condition\n");
        }
    }
    
    // k_sys_TerminalPrintf("blah\n");
    
    k_PIIX3_ISA_EndOfInterrupt(14);
}