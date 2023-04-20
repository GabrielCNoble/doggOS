#ifndef K_PIIX3_H
#define K_PIIX3_H

#include "../../pci.h"
#include "../../dsk.h"
#include "../../ps2.h"
#include "../../82C59.h"
#include "../../82C37.h"
#include "../../ide.h"
#include "../../dev.h"


#define K_PIIX3_PCI_TO_ISA_FUNCTION_INDEX 0
#define K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX 1

uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index, union k_pci_header_t *header);


#define K_PIIX3_IDE_CH0_PCI_IDETIM_REG 0x40
#define K_PIIX3_IDE_IRQ_VECTOR 14

#define K_PIIX3_PRIMARY_IDE_CMD_BLOCK 0x1f0

/* IDE interface */

uintptr_t k_PIIX3_IDE_Thread(void *data);

uint8_t k_PIIX3_IDE_ReadReg8(struct k_dev_ide_disk_t *disk, uint8_t reg);

uint16_t k_PIIX3_IDE_ReadReg16(struct k_dev_ide_disk_t *disk, uint8_t reg);

void k_PIIX3_IDE_ReadReg16S(struct k_dev_ide_disk_t *disk, uint8_t reg, uint32_t count, void *buffer);

void k_PIIX3_IDE_WriteReg8(struct k_dev_ide_disk_t *disk, uint8_t reg, uint8_t value);

void k_PIIX3_IDE_WriteReg16(struct k_dev_ide_disk_t *disk, uint8_t reg, uint16_t value);

void k_PIIX3_IDE_WriteReg16S(struct k_dev_ide_disk_t *disk, uint8_t reg, uint32_t count, void *buffer);

void k_PIIX3_IDE_IRQHandler(uint32_t irq_vector, void *data);

/* PS2 controller */

// #define K_PIIX3_PS2_KEYBOARD_IRQ_VECTOR 1
// #define K_PIIX3_PS2_MOUSE_IRQ_VECTOR 12

uint8_t k_PIIX3_PS2_ReadData(struct k_dev_ps2_device_t *device);

void k_PIIX3_PS2_WriteData(struct k_dev_ps2_device_t *device, uint8_t value);

uint8_t k_PIIX3_PS2_ReadCmd(struct k_dev_ps2_device_t *device);

void k_PIIX3_PS2_WriteCmd(struct k_dev_ps2_device_t *device, uint8_t value);

void k_PIIX3_PS2_Keyboard_IRQHandler(uint32_t irq_vector, void *data);

void k_PIIX3_PS2_Mouse_IRQHandler(uint32_t irq_vector, void *data);



/* ISA bus */

/* 82C59 interrupt controllers */

#define K_PIIX3_82C59_IRQ_BASE       128

#define K_PIIX3_82C59_CTRL1_ICW1_REG 0x20
#define K_PIIX3_82C59_CTRL1_ICW2_REG 0x21

#define K_PIIX3_82C59_CTRL1_ICW3_REG 0x21
#define K_PIIX3_82C59_CTRL1_ICW4_REG 0x21

#define K_PIIX3_82C59_CTRL2_ICW1_REG 0xa0
#define K_PIIX3_82C59_CTRL2_ICW2_REG 0xa1

#define K_PIIX3_82C59_CTRL2_ICW3_REG 0xa1
#define K_PIIX3_82C59_CTRL2_ICW4_REG 0xa1

#define K_PIIX3_82C59_CTRL1_OCW1_REG 0x21
#define K_PIIX3_82C59_CTRL1_OCW2_REG 0x20
#define K_PIIX3_82C59_CTRL1_OCW3_REG 0x20

#define K_PIIX3_82C59_CTRL2_OCW1_REG 0xa1
#define K_PIIX3_82C59_CTRL2_OCW2_REG 0xa0
#define K_PIIX3_82C59_CTRL2_OCW3_REG 0xa0

/* 82C37 dma controllers */
#define K_PIIX3_82C37_CH03_CMD_REG 0x08
#define K_PIIX3_82C37_CH47_CMD_REG 0xd0

#define K_PIIX3_82C37_CH03_MOD_REG 0x0b
#define K_PIIX3_82C37_CH47_MOD_REG 0xd6

#define K_PIIX3_82C37_CH03_REQ_REG 0x09
#define K_PIIX3_82C37_CH47_REQ_REG 0xd2

#define K_PIIX3_82C37_CH03_MASK_REG 0xa0
#define K_PIIX3_82C37_CH47_MASK_REG 0xd4

#define K_PIIX3_82C37_CH03_STATUS_REG 0x08
#define K_PIIX3_82C37_CH47_STATUS_REG 0xd0

#define K_PIIX3_82C37_CH0_BASE_CUR_ADDR_REG 0x00
#define K_PIIX3_82C37_CH1_BASE_CUR_ADDR_REG 0x02
#define K_PIIX3_82C37_CH2_BASE_CUR_ADDR_REG 0x04
#define K_PIIX3_82C37_CH3_BASE_CUR_ADDR_REG 0x06

#define K_PIIX3_82C37_CH4_BASE_CUR_ADDR_REG 0xc0
#define K_PIIX3_82C37_CH5_BASE_CUR_ADDR_REG 0xc4
#define K_PIIX3_82C37_CH6_BASE_CUR_ADDR_REG 0xc8
#define K_PIIX3_82C37_CH7_BASE_CUR_ADDR_REG 0xcc

#define K_PIIX3_82C37_CH0_BASE_CUR_COUNT_REG 0x01
#define K_PIIX3_82C37_CH1_BASE_CUR_COUNT_REG 0x03
#define K_PIIX3_82C37_CH2_BASE_CUR_COUNT_REG 0x05
#define K_PIIX3_82C37_CH3_BASE_CUR_COUNT_REG 0x07

#define K_PIIX3_82C37_CH4_BASE_CUR_COUNT_REG 0xc2
#define K_PIIX3_82C37_CH5_BASE_CUR_COUNT_REG 0xc6
#define K_PIIX3_82C37_CH6_BASE_CUR_COUNT_REG 0xca
#define K_PIIX3_82C37_CH7_BASE_CUR_COUNT_REG 0xce

#define K_PIIX3_82C37_CH0_LOW_PAGE_REG 0x87
#define K_PIIX3_82C37_CH1_LOW_PAGE_REG 0x83
#define K_PIIX3_82C37_CH2_LOW_PAGE_REG 0x81
#define K_PIIX3_82C37_CH3_LOW_PAGE_REG 0x82

#define K_PIIX3_82C37_CH5_LOW_PAGE_REG 0x8b
#define K_PIIX3_82C37_CH6_LOW_PAGE_REG 0x89
#define K_PIIX3_82C37_CH7_LOW_PAGE_REG 0x8a

#define K_PIIX3_82C37_CH03_CLEAR_BYTE_CMD_REG 0x0c
#define K_PIIX3_82C37_CH47_CLEAR_BYTE_CMD_REG 0xd8

#define K_PIIX3_82C37_CH03_CLEAR_MASK_CMD_REG 0x0e
#define K_PIIX3_82C37_CH47_CLEAR_MASK_CMD_REG 0xdc


// #define K_PIIX3_KEYBOARD_IRQ_VECTOR 1

uint16_t k_PIIX3_ISA_ReadIRReg();

void k_PIIX3_ISA_EndOfInterrupt(uint32_t irq_vector);

void k_PIIX3_ISA_Timer1_IRQHandler(uint32_t irq_vector, void *data);

#endif