#include "isa.h"
#include "../../drv/82C59.h"
#include "../../../cpu/k_cpu.h"
#include "../pci.h"

uint32_t k_PIIX3_ISA_Init(uint8_t bus_index, uint8_t device_index, uint8_t function_index)
{
    /* 82C59 interrupt controllers */
    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(32), K_PIIX3_82C59_CTRL1_ICW2_REG);
    k_cpu_OutB(K_82C59_M_ICW3(0, 0, 1, 0, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW4_REG);

    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL2_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(40), K_PIIX3_82C59_CTRL2_ICW2_REG);
    k_cpu_OutB(K_82C59_S_ICW3(2), K_PIIX3_82C59_CTRL2_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL2_ICW4_REG);
    
    // k_cpu_OutB(0x00, K_PIIX3_82C59_CTRL1_OCW1_REG);
    // k_cpu_OutB(0x00, K_PIIX3_82C59_CTRL2_OCW1_REG);
    // k_cpu_OutB(0x01, K_PIIX3_82C59_CTRL1_OCW2_REG);
    // k_cpu_OutB(0x60, K_PIIX3_82C59_CTRL1_OCW2_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW1_REG));
    // k_cpu_OutB(0xa, K_PIIX3_82C59_CTRL1_OCW3_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW3_REG));
    // k_cpu_OutB(0xb, K_PIIX3_82C59_CTRL1_OCW3_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW3_REG));
    
    
    /* 82C37 dma controllers */
    // k_cpu_OutB(K_82C37_CMD(1, 0, 0, 0, 0, 0, 0, 0), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(K_82C37_CMD(0, 0, 0, 0, 0, 0, 0, 0), K_PIIX3_82C37_CH47_MOD_REG);
    // k_cpu_OutB(K_82C37_MODE(0, 2, 0, 1, 2), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(K_82C37_MODE(1, 1, 0, 1, 2), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(0x7f, K_PIIX3_82C37_CH0_LOW_PAGE_REG);
    // k_cpu_OutB(0x7e, K_PIIX3_82C37_CH1_LOW_PAGE_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_MASK_CMD_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_BYTE_CMD_REG);
    // k_cpu_OutB(0x10, K_PIIX3_82C37_CH0_BASE_CUR_COUNT_REG);
    // // k_cpu_OutB(0x00, K_PIIX3_82C37_CH0_BASE_CUR_COUNT_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_BYTE_CMD_REG);
    // k_cpu_OutB(0x10, K_PIIX3_82C37_CH1_BASE_CUR_COUNT_REG);
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH1_BASE_CUR_COUNT_REG);
    // k_cpu_OutB(0x04, K_PIIX3_82C37_CH03_REQ_REG);
    
    
    uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, function_index, 0);
        
    /* enable mouse interrupts */
    uint16_t xbcs = k_pci_ReadWord(base_address, 0x4e) | 0x10;
    k_pci_WriteWord(base_address, 0x4e, xbcs);
    return 0;
}

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


