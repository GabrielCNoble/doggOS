#ifndef DRV_PIIX3_H
#define DRV_PIIX3_H

#include "../pci.h"

#define K_PIIX3_PCI_TO_ISA_FUNCTION_INDEX 0
#define K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX 1

/* 
====================================================================
82C37 DMA controller 
====================================================================
*/
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

/* 
====================================================================
82C59 interrupt controller 
====================================================================
*/


/* 
====================================================================
IDE interface 
====================================================================
*/

// #define K_PIIX3_IDE_CH0_PCI_IDETIM_REG 0x40
// 
// #define K_PIIX3_PRIMARY_IDE_CMD_BLOCK 0x1f0
// #define K_PIIX3_IDE_DATA_REG          0x00
// #define K_PIIX3_IDE_ERROR_REG         0x01
// #define K_PIIX3_IDE_SEC_COUNT_REG     0x02
// #define K_PIIX3_IDE_SEC_NUMBER_REG    0x03
// #define K_PIIX3_IDE_CYL_LOW_REG       0x04
// #define K_PIIX3_IDE_CYL_HIGH_REG      0x05
// #define K_PIIX3_IDE_DRIVE_REG         0x06
// #define K_PIIX3_IDE_HEAD_REG          0x06
// #define K_PIIX3_IDE_CMD_REG           0x07
// #define K_PIIX3_IDE_STATUS_REG        0x07




uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index);

// uint32_t k_PIIX3_F0_Init(struct k_pci_init_info_t *init_info);

// uint32_t k_PIIX3_F1_Init(struct k_pci_init_info_t *init_info);

// uint32_t k_PIIX3_F2_Init(struct k_pci_init_info_t *init_info);


// void k_PIIX3_82C37_SetMode(uint8_t channel, uint8_t mode);
// 
// uint8_t k_PIIX3_82C37_GetMode(uint8_t channel);
// 
// void k_PIIX3_82C37_RequestTransfer(uint8_t channel);



#endif