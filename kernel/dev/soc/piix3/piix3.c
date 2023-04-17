#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../io.h"
#include "../../../irq/irq.h"
#include "../../../mem/pmap.h"
#include "../../../mem/mngr.h"
#include "../../../rt/mem.h"
#include "../../../sys/term.h"
#include "../../../defs.h"
#include "../../pci.h"
// #include "../../dsk.h"

// #include "ide.h"
// #include "isa.h"
// #include "82C59.h"


// #include "../../../dsk/dsk.h"
// #include "../../../rt/mem.h"
// #include "../../../sys/term.h"


/* IDE interface */
// struct k_io_stream_t *k_PIIX3_IDE_stream;
// extern void *k_PIIX3_IDE_Handler_a;
// extern void *k_PIIX3_ISA_Timer1_Handler_a;
// extern void *k_PIIX3_PS2_KeyboardHandler_a;
// extern void *k_PIIX3_PS2_MouseHandler_a;
// extern void *k_dev_kb_KeyboardHandler_a;

// uint32_t K_PIIX3_IDE_total_transferred;
struct k_dsk_cmd_t *k_PIIX3_IDE_current_cmd;
// struct k_dsk_disk_t *k_PIIX3_IDE_disk;
/* TODO: this shouldn't be a global... */
struct k_dev_ide_disk_t *   k_PIIX3_IDE_disk;
struct k_dev_ide_state_t    k_PIIX3_IDE_state;
// struct k_ide_cmd_state_t k_PIIX3_IDE_cmd_state;
// struct k_ide_device_t k_PIIX3_IDE_device;


uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index, union k_pci_header_t *header)
{   
    (void)bus_index;
    (void)device_index;

    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE, k_PIIX3_ISA_Timer1_IRQHandler);

    /* 82C59 interrupt controllers */
    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(K_PIIX3_82C59_IRQ_BASE), K_PIIX3_82C59_CTRL1_ICW2_REG);
    k_cpu_OutB(K_82C59_M_ICW3(0, 0, 1, 0, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW4_REG);

    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL2_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(K_PIIX3_82C59_IRQ_BASE + 8), K_PIIX3_82C59_CTRL2_ICW2_REG); 
    k_cpu_OutB(K_82C59_S_ICW3(2), K_PIIX3_82C59_CTRL2_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL2_ICW4_REG);

    
    struct k_dev_device_desc_t disk_def = {
        .device_size = sizeof(struct k_dev_ide_disk_t),
        .device_type = K_DEV_DEVICE_TYPE_DISK,
        .driver_func = k_PIIX3_IDE_Thread,
        .name        = "PIIX3 IDE Interface" 
    };

    k_PIIX3_IDE_disk = (struct k_dev_ide_disk_t *)k_dev_CreateDevice(&disk_def);

    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_PIIX3_IDE_IRQ_VECTOR, k_PIIX3_IDE_IRQHandler);

    k_PIIX3_IDE_disk->funcs.ReadReg8 = k_PIIX3_IDE_ReadReg8;
    k_PIIX3_IDE_disk->funcs.ReadReg16 = k_PIIX3_IDE_ReadReg16;
    k_PIIX3_IDE_disk->funcs.ReadReg16S = k_PIIX3_IDE_ReadReg16S;
    k_PIIX3_IDE_disk->funcs.WriteReg8 = k_PIIX3_IDE_WriteReg8;
    k_PIIX3_IDE_disk->funcs.WriteReg16 = k_PIIX3_IDE_WriteReg16;
    k_PIIX3_IDE_disk->funcs.WriteReg16S = k_PIIX3_IDE_WriteReg16S;

    k_PIIX3_IDE_disk->disk.read = k_IDE_Read;
    k_PIIX3_IDE_disk->disk.write = NULL;
    k_PIIX3_IDE_disk->disk.clear = NULL;
    k_PIIX3_IDE_disk->disk.type = K_DEV_DSK_TYPE_DISK;
    k_PIIX3_IDE_disk->disk.data = NULL;
    k_PIIX3_IDE_disk->disk.cur_cmd_page = k_rt_Malloc(sizeof(struct k_dev_dsk_cmd_page_t), 0);
    k_PIIX3_IDE_disk->disk.freeable_cmd_pages = NULL;
    k_PIIX3_IDE_disk->disk.cmd_pages = NULL;
    k_PIIX3_IDE_disk->disk.last_cmd_page = NULL;
    k_PIIX3_IDE_disk->disk.cmd_page_count = 0;
    k_PIIX3_IDE_disk->disk.cmd_page_index = 0;
    k_PIIX3_IDE_disk->disk.cmd_size = sizeof(struct k_dev_dsk_ide_cmd_t);
    k_PIIX3_IDE_disk->disk.cmd_align = alignof(struct k_dev_dsk_ide_cmd_t);

    k_dev_InitDiskCmdPage((struct k_dev_disk_t *)k_PIIX3_IDE_disk, k_PIIX3_IDE_disk->disk.cur_cmd_page);
    k_IDE_Identify(k_PIIX3_IDE_disk);
    k_dev_StartDevice((struct k_dev_device_t *)k_PIIX3_IDE_disk);

    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_PIIX3_PS2_KEYBOARD_IRQ_VECTOR, k_PIIX3_PS2_Keyboard_IRQHandler);
    // k_int_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_PIIX3_PS2_KEYBOARD_IRQ_VECTOR, &k_PIIX3_PS2_KeyboardHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3);
    // k_int_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_PIIX3_PS2_MOUSE_IRQ_VECTOR, &k_PIIX3_PS2_MouseHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3);

    // struct k_dev_dsk_ide_cmd_t id_cmd = {};
    // struct k_ide_info_t ide_info = {};
    // id_cmd.cmd.type = K_DEV_DSK_CMD_TYPE_IDENTIFY;
    // id_cmd.cmd.buffer = &ide_info;
    // id_cmd.cmd.size = sizeof(struct k_ide_info_t);
    // id_cmd.cmd.address = 0;
    // id_cmd.skip_count = 0;
    // id_cmd.sector_count = 1;

    // k_PIIX3_IDE_disk->cmd = &id_cmd;
    // /* necessary so the interrupt handler works properly. This is is fine
    // because the identify command doesn't really rely on the block size. */
    // k_PIIX3_IDE_disk->disk.block_size = 512;
    
    // k_PIIX3_IDE_Identify((struct k_dev_dsk_cmd_t *)&id_cmd);
    // k_cpu_EnableInterrupts();
    // while(!id_cmd.cmd.condition);
    // k_cpu_DisableInterrupts();
    // // k_proc_WaitCondition(&id_cmd.cmd.condition);
    
    // uint32_t lba_sector_count = (((uint32_t)ide_info.lba_sector_count[0]) << 16) | ((uint32_t)ide_info.lba_sector_count[1]);

    // k_PIIX3_IDE_disk->disk.block_size = ide_info.bytes_per_sector;
    // k_PIIX3_IDE_disk->disk.block_count = lba_sector_count;
    // k_PIIX3_IDE_disk->disk.start_address = 0;

    // uint16_t reg_value = k_pci_ReadWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0), 0xa2);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)reg_value);
    // k_cpu_Halt();
    // reg_value |= (1 << 4) | 1;
    // k_pci_WriteWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0), 0xa2, reg_value);

    uint16_t reg_value = k_pci_ReadWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0x4e), 0);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)reg_value);
    // k_cpu_Halt();
    reg_value |= (1 << 4);
    k_pci_WriteWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0x4e), 0, reg_value);
    
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

uint8_t k_PIIX3_IDE_ReadReg8(struct k_dev_ide_disk_t *disk, uint8_t reg)
{
    (void)disk;
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg);
}

uint16_t k_PIIX3_IDE_ReadReg16(struct k_dev_ide_disk_t *disk, uint8_t reg)
{
    (void)disk;
    return k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg);
}

void k_PIIX3_IDE_ReadReg16S(struct k_dev_ide_disk_t *disk, uint8_t reg, uint32_t count, void *buffer)
{
    (void)disk;
    k_cpu_InSW(count, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg, buffer);
}

void k_PIIX3_IDE_WriteReg8(struct k_dev_ide_disk_t *disk, uint8_t reg, uint8_t value)
{
    (void)disk;
    k_cpu_OutB(value, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg);
}

void k_PIIX3_IDE_WriteReg16(struct k_dev_ide_disk_t *disk, uint8_t reg, uint16_t value)
{
    (void)disk;
    k_cpu_OutW(value, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg);
}

void k_PIIX3_IDE_WriteReg16S(struct k_dev_ide_disk_t *disk, uint8_t reg, uint32_t count, void *buffer)
{
    (void)disk;
    (void)reg;
    (void)count;
    (void)buffer;
    // k_cpu_OutSW(count, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + reg);
}

void k_PIIX3_IDE_IRQHandler(uint32_t irq_vector)
{
    __asm__ volatile 
    (
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );
    k_IDE_InterruptHandler(k_PIIX3_IDE_disk);
    k_PIIX3_ISA_EndOfInterrupt(K_PIIX3_IDE_IRQ_VECTOR);
}

// uint8_t k_PIIX3_IDE_ReadStatus()
// {
//     // return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_STATUS);
//     return k_PIIX3_IDE_disk->funcs.ReadReg8(k_PIIX3_IDE_disk, K_IDE_CMD_REG_STATUS);
// }

// uint8_t K_PIIX3_IDE_ReadError()
// {
//     // return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_ERROR);
//     return k_PIIX3_IDE_disk->funcs.ReadReg8(k_PIIX3_IDE_disk, K_IDE_CMD_REG_ERROR);
// }

// void k_PIIX3_IDE_ExecCmd(uint32_t cmd)
// {
//     while(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_BSY);
//     k_PIIX3_IDE_disk->funcs.WriteReg8(k_PIIX3_IDE_disk, K_IDE_CMD_REG_CMD, cmd);
//     // k_cpu_OutB(cmd, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CMD);
// }

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

// uint32_t k_PIIX3_IDE_Read(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd)
// {    
//     // struct k_ide_cmd_state_t *cmd_state = (struct k_ide_cmd_state_t *)disk->data;
//     struct k_dev_ide_disk_t *ide_disk = (struct k_dev_ide_disk_t *)disk;
//     struct k_dev_dsk_ide_cmd_t *ide_cmd = (struct k_dev_dsk_ide_cmd_t *)cmd;
//     uint32_t block_size = k_PIIX3_IDE_disk->disk.block_size;
//     uint32_t first_block = cmd->address / block_size;
//     uint32_t last_block = ((ide_cmd->cmd.address + ide_cmd->cmd.size + block_size - 1) & (~(block_size - 1))) / block_size;
//     uint32_t block_count = last_block - first_block;

//     // k_sys_TerminalPrintf("k_PIIX3_IDE_Read: address: %d, size: %d, block size: %d, block count: %d\n", ide_cmd->cmd.address, ide_cmd->cmd.size, block_size, block_count);


//     if(block_count)
//     {
//         /* keep only the byte offset into the sector. This will be used for handling not 
//         sector/word aligned start addresses, and also as a total bytes copied counter */
//         ide_cmd->skip_count = ide_cmd->cmd.address & 0x1ff;
//         // cmd_state->cur_cmd = cmd;
//         ide_cmd->cmd.address = 0;
//         ide_disk->cmd = ide_cmd;
//         // disk->data = ide_cmd;

//         // k_sys_TerminalPrintf("k_PIIX3_IDE_Read: read %d sectors\n", block_count);

//         while(block_count > 0)
//         {
//             uint32_t read_block_count = block_count;

//             if(read_block_count > 256)
//             {
//                 read_block_count = 256;
//             }

//             ide_cmd->transfer_condition = 0;
//             ide_cmd->sector_count = read_block_count;

//             // uint8_t status = 0;
//             // do
//             // {
//             //     status = k_PIIX3_IDE_ReadStatus();
//             //     k_sys_TerminalPrintf("status: %x\n", (uint32_t)status);
//             // }
//             // while(!(status & K_IDE_STATUS_FLAG_DRDY) || (status & K_IDE_STATUS_FLAG_BSY));

//             while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));

//             // k_cpu_OutB((first_block & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_NUMBER);
//             // k_cpu_OutB(((first_block >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_LOW);
//             // k_cpu_OutB(((first_block >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_CYL_HI);
//             // k_cpu_OutB(((first_block >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DRV_HEAD);
//             // k_cpu_OutB(read_block_count, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_SEC_COUNT);

//             ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_SEC_NUMBER, first_block & 0xff);
//             ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_CYL_LOW, (first_block >> 8) & 0xff);
//             ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_CYL_HI, (first_block >> 16) & 0xff);
//             ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_DRV_HEAD, ((first_block >> 24) & 0x0f) | 0xe0);
//             ide_disk->funcs.WriteReg8(ide_disk, K_IDE_CMD_REG_SEC_COUNT, read_block_count);

//             k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
//             k_proc_WaitCondition(&ide_cmd->transfer_condition);
//             block_count -= read_block_count;
//             first_block += read_block_count;
//         }
//     }
//     else
//     {
//         k_rt_SignalCondition(&ide_cmd->cmd.condition);
//     }
    
//     return 0;
// }

// uint32_t k_PIIX3_IDE_Identify(struct k_dev_dsk_cmd_t *cmd)
// {
//     while(!(k_IDE_ReadStatus(k_PIIX3_IDE_disk) & K_IDE_STATUS_FLAG_DRDY));
//     k_IDE_ExecCmd(k_PIIX3_IDE_disk, K_IDE_CMD_ID_DRIVE);

//     return 0;
// }

// void k_PIIX3_IDE_Handler(struct k_dev_disk_t *disk)
// {
//     struct k_dev_ide_disk_t *ide_disk = (struct k_dev_ide_disk_t *)disk;
//     struct k_dev_dsk_ide_cmd_t *ide_cmd = ide_disk->cmd;
//     // k_sys_TerminalPrintf("a\n");
    
//     // uint16_t temp_data[32];
//     if(k_IDE_ReadStatus(ide_disk) & K_IDE_STATUS_FLAG_ERROR)
//     {
//         // uint8_t error = K_PIIX3_IDE_ReadError();
        
//         k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)k_IDE_ReadError(ide_disk));
//         k_rt_SignalCondition(&ide_cmd->cmd.condition);
//     }
//     else
//     {        
//         uint32_t available_bytes = ide_disk->disk.block_size;
//         // k_cpu_Halt();
//         // k_sys_TerminalPrintf("available_bytes: %d\n", available_bytes); 
//         switch(ide_cmd->cmd.type)
//         {
//             case K_DEV_DSK_CMD_TYPE_WRITE:
//                 // if(cmd->address)
//                 // {
//                 //     /* address is not sector aligned, so we'll just throw out the data we don't need */
//                 //     uint32_t word_count = (cmd->address & 0x1ff) / sizeof(uint16_t);
//                 //     for(uint32_t index = 0; index < word_count; index++)
//                 //     {
//                 //         // k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //         k_cpu_OutW(0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //     }
//                 //     cmd->address -= word_count * sizeof(uint16_t);
//                 // 
//                 //     if(cmd->address & 0x1)
//                 //     {
//                 //         /* address is not word aligned, so we'll read a word here and discard half of it */
//                 //         // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //         uint16_t data = ((uint8_t *)cmd->buffer)[0];
//                 //         data <<= 8;
//                 //         k_cpu_OutW(data, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //         // ((uint8_t *)cmd->buffer)[0] = (uint8_t)(data >> 8);
//                 //         cmd->address--;
//                 //         cmd->size--;
//                 //     }
//                 // }
//                 // 
//                 // if(cmd->size > 1)
//                 // {
//                 //     /* bulk of the copy  */
//                 //     uint32_t copy_size = 512 / sizeof(uint16_t);
//                 //     uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
//                 // 
//                 //     if(copy_size > buffer_word_count)
//                 //     {
//                 //         copy_size = buffer_word_count;
//                 //     }
//                 // 
//                 //     copy_size /= sizeof(temp_data) / sizeof(temp_data[0]);
//                 //     for(uint32_t index = 0; index < copy_size; index++)
//                 //     {
//                 //         // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //         k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG, temp_data, sizeof(temp_data) / sizeof(temp_data[0]));
//                 //         k_rt_CopyBytes((uint8_t *)cmd->buffer + cmd->address, temp_data, sizeof(temp_data));
//                 //         cmd->address += sizeof(temp_data);
//                 //         cmd->size -= sizeof(temp_data);
//                 //     }
//                 // }
//                 // else if(cmd->size == 1)
//                 // {
//                 //     /* the size of the buffer is not a round number of words, so copy the last byte here */
//                 //     uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
//                 //     ((uint8_t *)cmd->buffer)[cmd->address] = (uint8_t)data;
//                 //     cmd->size--;
//                 // }
//             break;
            
//             case K_DEV_DSK_CMD_TYPE_READ:
//             case K_DEV_DSK_CMD_TYPE_IDENTIFY:
//                 // uint32_t bytes_left = cmd->size - cmd->address;
//                 // k_sys_TerminalPrintf("0\n");
//                 if(ide_cmd->skip_count)
//                 {
//                     /* copy is not sector aligned, so we'll just throw out the data we don't need */
//                     uint32_t word_count = ide_cmd->skip_count / sizeof(uint16_t);
//                     for(uint32_t index = 0; index < word_count; index++)
//                     {
//                         // k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
//                         ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
//                     }

//                     available_bytes -= word_count * sizeof(uint16_t);
                    
                    
//                     if(ide_cmd->skip_count & 0x1)
//                     {
//                         /* address is not word aligned, so we'll read a word here and discard the first half */
//                         // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
//                         uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
//                         /* address is pointing at the second byte of a word */
//                         ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)(data >> 8);
//                         ide_cmd->cmd.address++;
//                         available_bytes -= 2;
//                     }

//                     ide_cmd->skip_count = 0;

//                     // k_sys_TerminalPrintf("1\n");

//                     if(!available_bytes)
//                     {
//                         /* we actually read a whole sector from the sector buffer, so get out */
//                         break;
//                     }
//                 }
                
//                 if(available_bytes > 1)
//                 {
//                     /* bulk of the copy  */
//                     uint32_t copy_size = available_bytes / sizeof(uint16_t);
//                     uint32_t buffer_word_count = ide_cmd->cmd.size >> 1;
//                     // uint32_t buffer_word_count = cmd->size / sizeof(uint16_t);
                    
//                     if(copy_size > buffer_word_count)
//                     {
//                         copy_size = buffer_word_count;
//                     }

//                     if(ide_cmd->cmd.address & 1)
//                     {
//                         /* Well, shit, unaligned copy */
//                         for(uint32_t word_index = 0; word_index < copy_size; word_index++)
//                         {
//                             // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
//                             uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
//                             ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)data;
//                             ide_cmd->cmd.address++;
//                             ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)(data >> 8);
//                             ide_cmd->cmd.address++;
//                         }
//                     }
//                     else
//                     {
//                         /* Cool, fast(er) aligned copy */
//                         uint16_t *buffer = (uint16_t *)((uint8_t *)ide_cmd->cmd.buffer + ide_cmd->cmd.address);
//                         // k_cpu_InSW(copy_size, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA, buffer);
//                         ide_disk->funcs.ReadReg16S(ide_disk, K_IDE_CMD_REG_DATA, copy_size, buffer);
//                         ide_cmd->cmd.address += copy_size * sizeof(uint16_t);                    
//                     }
                    
//                     available_bytes -= copy_size * sizeof(uint16_t);

//                     // k_sys_TerminalPrintf("2\n");
//                     // cmd->size -= copy_size * sizeof(uint16_t);
//                 }
                
//                 if(available_bytes == 1)
//                 {
//                     /* the size of the buffer is not a round number of words, so copy the last byte here */
//                     // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
//                     uint16_t data = ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
//                     ((uint8_t *)ide_cmd->cmd.buffer)[ide_cmd->cmd.address] = (uint8_t)data;
//                     ide_cmd->cmd.address++;

//                     // k_sys_TerminalPrintf("3\n");
//                 }
//                 else
//                 {
//                     /* apparently the IDE controller expects us to read a whole sector...? Not
//                     doing so causes the kernel to hang on qemu. Works fine on bochs, though. The
//                     AT attachment spec says the host reads a sector of data during pio transfers, 
//                     but doesn't seem to assert it's absolutely required to read the whole sector
//                     buffer. */
//                     uint32_t word_count = available_bytes / sizeof(uint16_t);
//                     // k_sys_TerminalPrintf("word count: %d\n", word_count);
//                     for(uint32_t word_index = 0; word_index < word_count; word_index++)
//                     {
//                         // k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_IDE_CMD_REG_DATA);
//                         ide_disk->funcs.ReadReg16(ide_disk, K_IDE_CMD_REG_DATA);
//                     }
//                 }
//             break;
//         }

        

//         ide_cmd->sector_count--;
        
//         if(ide_cmd->cmd.address == ide_cmd->cmd.size)
//         {
//             /* let anything waiting for this command to know we're done */
//             // k_sys_TerminalPrintf("done...\n");
//             k_rt_SignalCondition(&ide_cmd->cmd.condition);
//         }

//         if(ide_cmd->sector_count == 0)
//         {
//             k_rt_SignalCondition(&ide_cmd->transfer_condition);
//         }
//     }
    
//     // k_PIIX3_ISA_EndOfInterrupt(14);

//     // if(ide_cmd->cmd.type == K_DEV_DSK_CMD_TYPE_READ)
//     // {
//     //     k_sys_TerminalPrintf("sector count: %d\n", (uint32_t)ide_cmd->sector_count);
//     // }
// }

uintptr_t k_PIIX3_IDE_Thread(void *data)
{
    // k_int_SetInterruptHandler(K_PIIX3_IDE_IRQ_VECTOR, (uintptr_t)&k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);

    struct k_dev_ide_disk_t *disk = (struct k_dev_ide_disk_t *)data;

    // struct k_dev_dsk_ide_cmd_t id_cmd = {};
    // struct k_ide_info_t ide_info = {};
    // id_cmd.cmd.type = K_DEV_DSK_CMD_TYPE_IDENTIFY;
    // id_cmd.cmd.buffer = &ide_info;
    // id_cmd.cmd.size = sizeof(struct k_ide_info_t);
    // id_cmd.cmd.address = 0;
    // id_cmd.skip_count = 0;
    // id_cmd.sector_count = 1;

    // disk->cmd = &id_cmd;
    // /* necessary so the interrupt handler works properly. This is is fine
    // because the identify command doesn't really rely on the block size. */
    // disk->disk.block_size = 512;
    
    // k_PIIX3_IDE_Identify((struct k_dev_dsk_cmd_t *)&id_cmd);
    // k_proc_WaitCondition(&id_cmd.cmd.condition);
    
    // uint32_t lba_sector_count = (((uint32_t)ide_info.lba_sector_count[0]) << 16) | ((uint32_t)ide_info.lba_sector_count[1]);

    // disk->disk.block_size = ide_info.bytes_per_sector;
    // disk->disk.block_count = lba_sector_count;
    // disk->disk.start_address = 0;

    return k_dev_DiskThread(data);
}

/* PS2 controller */

uint8_t k_PIIX3_PS2_ReadData(struct k_dev_ps2_device_t *device)
{
    return k_cpu_InB(K_DEV_PS2_DATA_PORT);
}

void k_PIIX3_PS2_WriteData(struct k_dev_ps2_device_t *device, uint8_t value)
{
    return k_cpu_OutB(value, K_DEV_PS2_DATA_PORT);
}

uint8_t k_PIIX3_PS2_ReadCmd(struct k_dev_ps2_device_t *device)
{
    return k_cpu_InB(K_DEV_PS2_STATUS_CMD_PORT);
}

void k_PIIX3_PS2_WriteCmd(struct k_dev_ps2_device_t *device, uint8_t value)
{
    return k_cpu_OutB(value, K_DEV_PS2_STATUS_CMD_PORT);
}

void k_PIIX3_PS2_Keyboard_IRQHandler(uint32_t irq_vector)
{
    (void)irq_vector;
    k_dev_kb_KeyboardHandler();
    k_PIIX3_ISA_EndOfInterrupt(K_PIIX3_PS2_KEYBOARD_IRQ_VECTOR);
}

/* ISA bus */

uint16_t k_PIIX3_ISA_ReadIRReg()
{
    
}

void k_PIIX3_ISA_EndOfInterrupt(uint32_t irq_vector)
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

void k_PIIX3_ISA_Timer1_IRQHandler(uint32_t irq_vector)
{
    (void)irq_vector;
    k_PIIX3_ISA_EndOfInterrupt(0);
}