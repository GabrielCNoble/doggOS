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
// struct k_dev_ide_disk_t *   k_PIIX3_IDE_disk;
struct k_dev_ide_state_t    k_PIIX3_IDE_state;
// struct k_ide_cmd_state_t k_PIIX3_IDE_cmd_state;
// struct k_ide_device_t k_PIIX3_IDE_device;


uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index, union k_pci_header_t *header)
{   
    (void)bus_index;
    (void)device_index;

    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE, k_PIIX3_ISA_Timer1_IRQHandler, NULL);

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

    struct k_dev_ide_disk_t *disk = (struct k_dev_ide_disk_t *)k_dev_CreateDevice(&disk_def);

    disk->funcs.ReadReg8 = k_PIIX3_IDE_ReadReg8;
    disk->funcs.ReadReg16 = k_PIIX3_IDE_ReadReg16;
    disk->funcs.ReadReg16S = k_PIIX3_IDE_ReadReg16S;
    disk->funcs.WriteReg8 = k_PIIX3_IDE_WriteReg8;
    disk->funcs.WriteReg16 = k_PIIX3_IDE_WriteReg16;
    disk->funcs.WriteReg16S = k_PIIX3_IDE_WriteReg16S;

    disk->disk.read = k_IDE_Read;
    disk->disk.write = NULL;
    disk->disk.clear = NULL;
    disk->disk.type = K_DEV_DSK_TYPE_DISK;
    disk->disk.data = NULL;
    disk->disk.cur_cmd_page = k_rt_Malloc(sizeof(struct k_dev_dsk_cmd_page_t), 0);
    disk->disk.freeable_cmd_pages = NULL;
    disk->disk.cmd_pages = NULL;
    disk->disk.last_cmd_page = NULL;
    disk->disk.cmd_page_count = 0;
    disk->disk.cmd_page_index = 0;
    disk->disk.cmd_size = sizeof(struct k_dev_dsk_ide_cmd_t);
    disk->disk.cmd_align = alignof(struct k_dev_dsk_ide_cmd_t);

    k_dev_InitDiskCmdPage((struct k_dev_disk_t *)disk, disk->disk.cur_cmd_page);
    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_PIIX3_IDE_IRQ_VECTOR, k_PIIX3_IDE_IRQHandler, disk);
    k_IDE_Identify(disk);
    k_dev_StartDevice((struct k_dev_device_t *)disk);

    struct k_dev_device_desc_t keyboard_desc = {
        .device_size = sizeof(struct k_dev_ps2_device_t),
        .device_type = K_DEV_DEVICE_TYPE_KEYBOARD,
        .driver_func = NULL,
        .name        = "PIIX3 PS2 Keyboard"
    };

    struct k_dev_ps2_device_t *ps2_device = k_dev_CreateDevice(&keyboard_desc);
    ps2_device->keyboard.funcs.ReadCmd = k_PIIX3_PS2_ReadCmd;
    ps2_device->keyboard.funcs.ReadData = k_PIIX3_PS2_ReadData;
    ps2_device->keyboard.funcs.WriteCmd = k_PIIX3_PS2_WriteCmd;
    ps2_device->keyboard.funcs.WriteData = k_PIIX3_PS2_WriteData;

    k_irq_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_DEV_PS2_KEYBOARD_IRQ_VECTOR, k_PIIX3_PS2_Keyboard_IRQHandler, ps2_device);
    k_dev_StartDevice((struct k_dev_device_t *)ps2_device);

    // uint16_t reg_value = k_pci_ReadWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0x4e), 0);
    // reg_value |= (1 << 4);
    // k_pci_WriteWord(K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, 0, 0x4e), 0, reg_value);
    
    return K_STATUS_OK;
}

/* IDE interface */

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

void k_PIIX3_IDE_IRQHandler(uint32_t irq_vector, void *data)
{
    struct k_dev_ide_disk_t *disk = (struct k_dev_ide_disk_t *)data;
    k_IDE_InterruptHandler(disk);
    k_PIIX3_ISA_EndOfInterrupt(K_PIIX3_IDE_IRQ_VECTOR);
}

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
    (void)device;
    return k_cpu_InB(K_DEV_PS2_DATA_PORT);
}

void k_PIIX3_PS2_WriteData(struct k_dev_ps2_device_t *device, uint8_t value)
{
    (void)device;
    return k_cpu_OutB(value, K_DEV_PS2_DATA_PORT);
}

uint8_t k_PIIX3_PS2_ReadCmd(struct k_dev_ps2_device_t *device)
{
    (void)device;
    return k_cpu_InB(K_DEV_PS2_STATUS_CMD_PORT);
}

void k_PIIX3_PS2_WriteCmd(struct k_dev_ps2_device_t *device, uint8_t value)
{
    (void)device;
    return k_cpu_OutB(value, K_DEV_PS2_STATUS_CMD_PORT);
}

void k_PIIX3_PS2_Keyboard_IRQHandler(uint32_t irq_vector, void *data)
{
    (void)irq_vector;
    struct k_dev_ps2_keyboard_t *keyboard = (struct k_dev_ps2_keyboard_t *)data;
    k_PS2_KeyboardInterruptHandler(keyboard);
    k_PIIX3_ISA_EndOfInterrupt(K_DEV_PS2_KEYBOARD_IRQ_VECTOR);
}

void k_PIIX3_PS2_Mouse_IRQHandler(uint32_t irq_vector, void *data)
{

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

void k_PIIX3_ISA_Timer1_IRQHandler(uint32_t irq_vector, void *data)
{
    (void)irq_vector;
    (void)data;
    k_PIIX3_ISA_EndOfInterrupt(0);
}