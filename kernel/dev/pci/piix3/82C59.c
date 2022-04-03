#include "82C59.h"

#include "../../drv/82C59.h"
#include "../../../cpu/k_cpu.h"

uint32_t k_PIIX3_82C59_Init(uint8_t bus_index, uint8_t device_index)
{
    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(32), K_PIIX3_82C59_CTRL1_ICW2_REG);
    k_cpu_OutB(K_82C59_M_ICW3(0, 0, 1, 0, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW4_REG);
    
    k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL2_ICW1_REG);
    k_cpu_OutB(K_82C59_ICW2(40), K_PIIX3_82C59_CTRL2_ICW2_REG);
    k_cpu_OutB(K_82C59_S_ICW3(2), K_PIIX3_82C59_CTRL2_ICW3_REG);
    k_cpu_OutB(K_82C59_ICW4(1, 0, 0, 0, 0), K_PIIX3_82C59_CTRL2_ICW4_REG);
    
    // k_PIIX3_82C59_EndOfInterrupt();
    
    // k_cpu_OutB(0x01, K_PIIX3_82C59_CTRL1_OCW2_REG);
    // k_cpu_OutB(0x60, K_PIIX3_82C59_CTRL1_OCW2_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW1_REG));
    // k_cpu_OutB(0xa, K_PIIX3_82C59_CTRL1_OCW3_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW3_REG));
    // k_cpu_OutB(0xb, K_PIIX3_82C59_CTRL1_OCW3_REG);
    // k_sys_TerminalPrintf("%x\n", (uint32_t)k_cpu_InB(K_PIIX3_82C59_CTRL1_OCW3_REG));
    // k_cpu_Halt();
    
    return 0;
}

void k_PIIX3_82C59_EndOfInterrupt()
{
    k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL2_OCW2_REG);
    k_cpu_OutB(0x20, K_PIIX3_82C59_CTRL1_OCW2_REG);
}