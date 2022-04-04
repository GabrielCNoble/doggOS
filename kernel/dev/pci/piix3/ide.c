#include "ide.h"
#include "82C59.h"
#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../int/int.h"
#include "../../../io.h"
#include "../../../dsk/dsk.h"
#include "../../../rt/mem.h"
#include "../../../sys/term.h"

// struct k_io_stream_t *k_PIIX3_IDE_stream;
extern void *k_PIIX3_IDE_Handler_a;

// uint32_t K_PIIX3_IDE_total_transferred;
struct k_dsk_cmd_t *k_PIIX3_IDE_current_cmd;
struct k_dsk_disk_t *k_PIIX3_IDE_disk;

uint32_t k_PIIX3_IDE_Init(uint8_t bus_index, uint8_t device_index)
{
    // uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX, 0);
    (void)bus_index;
    (void)device_index;
    k_int_SetInterruptHandler(46, (uintptr_t)&k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);
    k_PIIX3_IDE_disk = k_dsk_CreateDisk(512, 1000, 0, k_PIIX3_IDE_Read, NULL);
    
    // k_PIIX3_IDE_stream = k_io_AllocStream();
    // k_io_UnblockStream(k_PIIX3_IDE_stream);
    return 0;
}

uint8_t k_PIIX3_IDE_ReadStatus()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_STATUS_REG);
}

uint8_t K_PIIX3_IDE_ReadError()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_ERROR_REG);
}

void k_PIIX3_IDE_ExecCmd(uint8_t cmd)
{
    while(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_BSY);
    k_cpu_OutB(cmd, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CMD_REG);
}

void k_PIIX3_IDE_Delay()
{
    for(uint32_t x = 0; x < 0xffff; x++);
}

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
    k_PIIX3_IDE_current_cmd = cmd;
    // K_PIIX3_IDE_total_transferred = 0;
    uint32_t lba = cmd->address >> 9;
    uint32_t size = cmd->size >> 9;
    
    if(cmd->size & 0x1ff)
    {
        size++;
    }
    
    /* keep only the byte offset into the sector. This will be used for 
    handling not sector/word aligned start addresses, and also as a total 
    bytes copied counter */
    cmd->address &= 0x1ff;
    
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    k_cpu_OutB((lba & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_NUMBER_REG);
    k_cpu_OutB(((lba >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_LOW_REG);
    k_cpu_OutB(((lba >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_HIGH_REG);
    k_cpu_OutB(((lba >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_HEAD_REG);
    k_cpu_OutB(size, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_COUNT_REG);
    k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
    
    return 0;
}

void k_PIIX3_IDE_Handler()
{
    uint16_t temp_data[32];
    if(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_ERROR)
    {
        k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)K_PIIX3_IDE_ReadError());
    }
    else
    {
        struct k_dsk_cmd_t *cmd = k_PIIX3_IDE_current_cmd;
        // uint32_t copy_size = 512 / sizeof(uint16_t);
        // uint32_t buffer_word_count = (cmd->size - K_PIIX3_IDE_total_transferred) / sizeof(uint16_t);
        // 
        // if(copy_size > buffer_word_count)
        // {
        //     copy_size = buffer_word_count;
        // }
        // 
        // copy_size /= sizeof(temp_data) / sizeof(temp_data[0]);
        
        switch(cmd->type)
        {
            case K_DSK_CMD_TYPE_READ:
                if(cmd->address)
                {
                    /* address is not sector aligned, so we'll just throw out the data we don't need */
                    uint32_t word_count = (cmd->address & 0x1ff) / sizeof(uint16_t);
                    for(uint32_t index = 0; index < word_count; index++)
                    {
                        k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                    }
                    cmd->address -= word_count * sizeof(uint16_t);
                    
                    if(cmd->address & 0x1)
                    {
                        /* address is not word aligned, so we'll read a word here and discard half of it */
                        uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
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
                    
                    copy_size /= sizeof(temp_data) / sizeof(temp_data[0]);
                    for(uint32_t index = 0; index < copy_size; index++)
                    {
                        // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                        k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG, temp_data, sizeof(temp_data) / sizeof(temp_data[0]));
                        k_rt_CopyBytes((uint8_t *)cmd->buffer + cmd->address, temp_data, sizeof(temp_data));
                        cmd->address += sizeof(temp_data);
                        cmd->size -= sizeof(temp_data);
                    }
                }
                else if(cmd->size == 1)
                {
                    /* the size of the buffer is not a round number of words, so copy the last byte here */
                    uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
                    ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
                    cmd->size--;
                }
            break;
        }
        
        if(!cmd->size)
        {
            k_rt_SignalCondition(&cmd->condition);
        }
    }
    
    // k_io_SignalStream(k_PIIX3_IDE_stream);
    
    k_PIIX3_82C59_EndOfInterrupt();
}