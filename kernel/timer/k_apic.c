#include "k_apic.h"
#include "../mem/mem.h"
#include "../k_int.h"

uint32_t k_apic_regs_base = 0xfee00000;

void k_apic_Init()
{
    k_mem_MapLinearAddress(k_apic_regs_base, k_apic_regs_base, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE);
    // k_apic_WriteReg(K_APIC_REG_LVT_CMCI, k_apic_ReadReg(K_APIC_REG_LVT_CMCI) | K_INT_HANDLER_CMCI);
    // k_apic_WriteReg(K_APIC_REG_LVT_THERM_SENSOR, k_apic_ReadReg(K_APIC_REG_LVT_THERM_SENSOR) | K_INT_HANDLER_THERM);
    // k_apic_WriteReg(K_APIC_REG_LVT_LINT0, k_apic_ReadReg(K_APIC_REG_LVT_LINT0) | K_INT_HANDLER_LINT0);
    // k_apic_WriteReg(K_APIC_REG_LVT_LINT1, k_apic_ReadReg(K_APIC_REG_LVT_LINT1) | K_INT_HANDLER_LINT1);
    // k_apic_WriteReg(K_APIC_REG_LVT_ERROR, k_apic_ReadReg(K_APIC_REG_LVT_ERROR) | K_INT_HANDLER_ERROR);
    // k_apic_WriteReg(K_APIC_REG_LVT_TIMER, (k_apic_ReadReg(K_APIC_REG_LVT_TIMER) | (K_INT_HANDLER_TIME_SLICE & 0xff) ) & (0xfff8ffff));
    // k_apic_WriteReg(K_APIC_REG_DIV_CONFIG, k_apic_ReadReg(K_APIC_REG_DIV_CONFIG) & (~0xb));
    // k_apic_WriteReg(K_APIC_REG_SPUR_INT_VEC, k_apic_ReadReg(K_APIC_REG_SPUR_INT_VEC) | 34);
}

void k_apic_WriteReg(uint32_t reg, uint32_t value)
{
    uint32_t *reg_addr = (uint32_t *)(k_apic_regs_base + reg);
    *reg_addr = value;
}

void k_apic_AndReg(uint32_t reg, uint32_t value)
{
    uint32_t *reg_addr = (uint32_t *)(k_apic_regs_base + reg);
    *reg_addr &= value;
}

void k_apic_OrReg(uint32_t reg, uint32_t value)
{
    uint32_t *reg_addr = (uint32_t *)(k_apic_regs_base + reg);
    *reg_addr |= value;
}

uint32_t k_apic_ReadReg(uint32_t reg)
{
    uint32_t *reg_addr = (uint32_t *)(k_apic_regs_base + reg);
    return *reg_addr;
}

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

void k_apic_EndOfInterrupt()
{
    k_apic_WriteReg(K_APIC_REG_EOI, 0);
}

void k_apic_FireIPInterrupt(uint16_t destination, uint8_t interrupt)
{
    uint32_t high_value = k_apic_ReadReg(K_APIC_REG_INT_CMD1);
    uint32_t low_value = k_apic_ReadReg(K_APIC_REG_INT_CMD0);

    if(destination != K_APIC_INT_DEST_SELF)
    {
        high_value |= ((uint32_t)(destination & 0xff)) << 24;
    }
    else
    {
        /* self destination shorthand */
        low_value = (low_value & (~K_APIC_INT_DEST_SHORTHAND_LAST)) | K_APIC_INT_DEST_SHORTHAND_SELF;
        high_value &= 0x00ffffff;
    }

    /* physical destination mode */
    low_value &= ~K_APIC_INT_DEST_MODE_LAST;
    /* fixed delivery mode */
    low_value = (low_value & (~K_APIC_INT_DELIVERY_MODE_LAST)) | K_APIC_INT_DELIVERY_MODE_FIXED;

    low_value |= (1 << 14);
    /* edge trigger mode */
    low_value &= ~K_APIC_INT_TRIGGER_MODE_LAST;
    low_value = (low_value & 0xffffff00) | interrupt;

    k_apic_WriteReg(K_APIC_REG_INT_CMD1, high_value);
    k_apic_WriteReg(K_APIC_REG_INT_CMD0, low_value);
}

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

void k_apic_StartTimer(uint32_t count)
{
    k_apic_WriteReg(K_APIC_REG_INIT_COUNT, count);
}

void k_apic_StopTimer()
{
    k_apic_WriteReg(K_APIC_REG_INIT_COUNT, 0);
}
