#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../io.h"
#include "../../../int/irq.h"
#include "../../../mem/pmap.h"
#include "../../../mem/mngr.h"
#include "../../../rt/mem.h"
#include "../../../sys/term.h"
#include "../../../defs.h"
// #include "../../dsk.h"
#include "../../drv/82C59.h"
#include "../../drv/82C37.h"
#include "../../drv/ide.h"
#include "../../dev.h"
// #include "ide.h"
// #include "isa.h"
// #include "82C59.h"


// #include "../../../dsk/dsk.h"
// #include "../../../rt/mem.h"
// #include "../../../sys/term.h"


/* IDE interface */
// struct k_io_stream_t *k_PIIX3_IDE_stream;
extern void *k_PIIX3_IDE_Handler_a;

// uint32_t K_PIIX3_IDE_total_transferred;
struct k_dsk_cmd_t *k_PIIX3_IDE_current_cmd;
// struct k_dsk_disk_t *k_PIIX3_IDE_disk;
/* TODO: this shouldn't be a global... */
struct k_dev_disk_t *k_PIIX3_IDE_disk;
struct k_ide_cmd_state_t k_PIIX3_IDE_cmd_state;
// struct k_ide_device_t k_PIIX3_IDE_device;


uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index)
{    
    // k_PIIX3_ISA_Init(bus_index, device_index, K_PIIX3_PCI_TO_ISA_FUNCTION_INDEX);
    // k_PIIX3_IDE_Init(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX);
    
    struct k_dev_device_desc_t disk_def = {
        .device_size = sizeof(struct k_dev_disk_t),
        .device_type = K_DEV_DEVICE_TYPE_DISK,
        .driver_func = k_PIIX3_IDE_Thread,
        .name        = "PIIX3 IDE Interface" 
    };

    k_PIIX3_IDE_disk = k_dev_CreateDevice(&disk_def);
    // k_PIIX3_IDE_disk->type = K_DEV_DSK_TYPE_DISK;
    // k_PIIX3_IDE_disk->block_size = 512;
    // k_PIIX3_IDE_disk->
    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();




    /* 82C59 interrupt controllers */
    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(32), K_PIIX3_82C59_CTRL1_ICW2_REG);
    k_cpu_OutB(K_82C59_M_ICW3(0, 0, 1, 0, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW4_REG);

    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL2_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(40), K_PIIX3_82C59_CTRL2_ICW2_REG);
    k_cpu_OutB(K_82C59_S_ICW3(2), K_PIIX3_82C59_CTRL2_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL2_ICW4_REG);

    
    return K_STATUS_OK;
}


// uint32_t k_PIIX3_IDE_Init(uint8_t bus_index, uint8_t device_index, uint8_t function_index)
// {
//     // uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX, 0);
//     (void)bus_index;
//     (void)device_index;
//     (void)function_index;
    
//     // // k_PIIX3_IDE_device.read_reg = 
    
//     // struct k_dsk_cmd_t id_cmd = {};
//     // struct k_ide_info_t ide_info = {};
//     // id_cmd.type = K_DSK_CMD_TYPE_IDENTIFY;
//     // id_cmd.buffer = &ide_info;
//     // id_cmd.size = sizeof(struct k_ide_info_t);
//     // id_cmd.address = 0;
//     // k_int_SetInterruptHandler(K_PIIX3_IDE_IRQ_VECTOR, (uintptr_t)&k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);

//     // struct k_dsk_disk_t temp_disk = {
//     //     .block_size = 512,
//     //     .data = &k_PIIX3_IDE_cmd_state
//     // };

//     // k_PIIX3_IDE_disk = &temp_disk;

//     // k_cpu_EnableInterrupts();
//     // k_PIIX3_IDE_Identify(&id_cmd);
//     // while(!id_cmd.condition);
//     // k_cpu_DisableInterrupts();
    
//     // uint32_t lba_sector_count = (((uint32_t)ide_info.lba_sector_count[0]) << 16) | ((uint32_t)ide_info.lba_sector_count[1]);

//     // struct k_dsk_disk_def_t disk_def = {
//     //     .block_size = ide_info.bytes_per_sector,
//     //     .block_count = lba_sector_count,
//     //     .start_address = 0,
//     //     .read_func = k_PIIX3_IDE_Read,
//     //     .write_func = NULL,
//     //     .type = K_DSK_TYPE_DISK
//     // };

//     // k_PIIX3_IDE_disk = k_dsk_CreateDisk(&disk_def);
//     // k_PIIX3_IDE_disk->data = &k_PIIX3_IDE_cmd_state;

//     // struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)k_PIIX3_IDE_disk->data;
//     // cmd_state->cur_cmd = NULL;
//     // cmd_state->skip_count = 0;
    
//     return 0;
// }

uint16_t k_PIIX3_IDE_ReadReg(uint8_t reg)
{
    return 0;
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

void k_PIIX3_IDE_ExecCmd(uint32_t cmd)
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

uint32_t k_PIIX3_IDE_Read(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd)
{
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    
    struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)disk->data;
    uint32_t block_size = k_PIIX3_IDE_disk->block_size;
    uint32_t first_block = cmd->address / block_size;
    uint32_t last_block = ((cmd->address + cmd->size + block_size - 1) & (~(block_size - 1))) / block_size;
    uint32_t block_count = last_block - first_block;

    if(block_count)
    {
        /* keep only the byte offset into the sector. This will be used for handling not 
        sector/word aligned start addresses, and also as a total bytes copied counter */
        cmd_state->skip_count = cmd->address & 0x1ff;
        cmd_state->cur_cmd = cmd;
        cmd->address = 0;
        
        k_cpu_OutB((first_block & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_NUMBER);
        k_cpu_OutB(((first_block >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_LOW);
        k_cpu_OutB(((first_block >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_HI);
        k_cpu_OutB(((first_block >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DRV_HEAD);
        k_cpu_OutB(block_count, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_COUNT);
        k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
    }
    else
    {
        k_rt_SignalCondition(&cmd->condition);
    }
    
    return 0;
}

uint32_t k_PIIX3_IDE_Identify(struct k_dev_dsk_cmd_t *cmd)
{
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)k_PIIX3_IDE_disk->data;
    cmd_state->cur_cmd = cmd;
    cmd_state->skip_count = 0;
    cmd->address = 0;
    k_PIIX3_IDE_ExecCmd(K_IDE_CMD_ID_DRIVE);

    return 0;
}

void k_PIIX3_IDE_Handler()
{
    // struct k_dsk_cmd_t *cmd = k_PIIX3_IDE_current_cmd;
    struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)k_PIIX3_IDE_disk->data;
    struct k_dev_dsk_cmd_t *cmd = cmd_state->cur_cmd;

    
    
    
    // uint16_t temp_data[32];
    if(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_ERROR)
    {
        // uint8_t error = K_PIIX3_IDE_ReadError();
        
        k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)K_PIIX3_IDE_ReadError());
        k_rt_SignalCondition(&cmd->condition);
    }
    else
    {        
        uint32_t available_bytes = k_PIIX3_IDE_disk->block_size;
        // k_cpu_Halt();
        // k_sys_TerminalPrintf("available_bytes: %d\n", available_bytes); 
        switch(cmd->type)
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
                if(cmd_state->skip_count)
                {
                    /* copy is not sector aligned, so we'll just throw out the data we don't need */
                    uint32_t word_count = cmd_state->skip_count / sizeof(uint16_t);
                    for(uint32_t index = 0; index < word_count; index++)
                    {
                        k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    }

                    available_bytes -= word_count * sizeof(uint16_t);
                    
                    
                    if(cmd_state->skip_count & 0x1)
                    {
                        /* address is not word aligned, so we'll read a word here and discard the first half */
                        uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                        /* address is pointing at the second byte of a word */
                        ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)(data >> 8);
                        cmd->address++;
                        available_bytes -= 2;
                    }

                    cmd_state->skip_count = 0;

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
                    uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
                    
                    if(copy_size > buffer_word_count)
                    {
                        copy_size = buffer_word_count;
                    }

                    if(cmd->address & 1)
                    {
                        /* Well, shit, unaligned copy */
                        for(uint32_t word_index = 0; word_index < copy_size; word_index++)
                        {
                            uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                            ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
                            cmd->address++;
                            ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)(data >> 8);
                            cmd->address++;
                        }
                    }
                    else
                    {
                        /* Cool, fast(er) aligned copy */
                        uint16_t *buffer = (uint16_t *)((uint8_t *)cmd->buffer + cmd->address);
                        k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA, buffer, copy_size);
                        cmd->address += copy_size * sizeof(uint16_t);                    
                    }
                    
                    available_bytes -= copy_size * sizeof(uint16_t);
                    // cmd->size -= copy_size * sizeof(uint16_t);
                }
                
                if(available_bytes == 1)
                {
                    /* the size of the buffer is not a round number of words, so copy the last byte here */
                    uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
                    cmd->address++;
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
                        k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
                    }
                }
            break;
        }
        
        if(cmd->address == cmd->size)
        {
            /* let anything waiting for this command to know we're done */
            k_rt_SignalCondition(&cmd->condition);
        }
    }
    
    k_PIIX3_ISA_EndOfInterrupt(14);
}

uintptr_t k_PIIX3_IDE_Thread(void *data)
{
    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();
    struct k_dev_disk_t *disk = (struct k_dev_disk_t *)data;

    struct k_dev_dsk_cmd_t id_cmd = {};
    struct k_ide_info_t ide_info = {};
    id_cmd.type = K_DEV_DSK_CMD_TYPE_IDENTIFY;
    id_cmd.buffer = &ide_info;
    id_cmd.size = sizeof(struct k_ide_info_t);
    id_cmd.address = 0;

    k_int_SetInterruptHandler(K_PIIX3_IDE_IRQ_VECTOR, (uintptr_t)&k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);


    struct k_dev_disk_t temp_disk = {
        .block_size = 512,
        .data = &k_PIIX3_IDE_cmd_state
    };

    k_PIIX3_IDE_disk = &temp_disk;
    
    k_PIIX3_IDE_Identify(&id_cmd);
    k_proc_WaitCondition(&id_cmd.condition);
    
    uint32_t lba_sector_count = (((uint32_t)ide_info.lba_sector_count[0]) << 16) | ((uint32_t)ide_info.lba_sector_count[1]);

    disk->block_size = ide_info.bytes_per_sector;
    disk->block_count = lba_sector_count;
    disk->start_address = 0;
    disk->read = k_PIIX3_IDE_Read;
    disk->write = NULL;
    disk->clear = NULL;
    disk->type = K_DEV_DSK_TYPE_DISK;
    disk->data = &k_PIIX3_IDE_cmd_state;

    k_sys_TerminalPrintf("disk has %d blocks of %d bytes\n", disk->block_count, disk->block_size);
    k_sys_TerminalPrintf("extra: %d\n", ide_info.valid_extra);

    k_PIIX3_IDE_disk = disk;

    struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)k_PIIX3_IDE_disk->data;
    cmd_state->cur_cmd = NULL;
    cmd_state->skip_count = 0;

    return k_dev_DiskThread(data);
}

/* ISA bus */

uint16_t k_PIIX3_ISA_ReadIRReg()
{
    
}

void k_PIIX3_ISA_EndOfInterrupt(uint8_t irq_vector)
{
    // k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL2_OCW2_REG);
    // k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL1_OCW2_REG);
    
    k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL1_OCW2_REG);
    
    if(irq_vector > 7)
    {    
        k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL2_OCW2_REG);
    }
    // else
    // {
    //     k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL1_OCW2_REG);
    // }
}