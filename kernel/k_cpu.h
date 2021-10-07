#ifndef K_CPU_H
#define K_CPU_H

#include <stdint.h>
#include "k_defs.h"

enum K_CPU_STATUS_FLAGS
{
    K_CPU_STATUS_FLAG_CARRY = 1,
    K_CPU_STATUS_FLAG_PARITY = 1 << 2,
    K_CPU_STATUS_FLAG_AUX_CARRY = 1 << 4,
    K_CPU_STATUS_FLAG_ZERO = 1 << 6,
    K_CPU_STATUS_FLAG_SIGN = 1 << 7,
    K_CPU_STATUS_FLAG_TRAP = 1 << 8,
    K_CPU_STATUS_FLAG_INT_ENABLE = 1 << 9,
    K_CPU_STATUS_FLAG_DIRECTION = 1 << 10,
    K_CPU_STATUS_FLAG_OVERFLOW = 1 << 11,
    K_CPU_STATUS_FLAG_IOP_LEVEL0 = 0 << 12,
    K_CPU_STATUS_FLAG_IOP_LEVEL1 = 1 << 12,
    K_CPU_STATUS_FLAG_IOP_LEVEL2 = 2 << 12,
    K_CPU_STATUS_FLAG_IOP_LEVEL3 = 3 << 12,
    K_CPU_STATUS_FLAG_NESTED_TASK = 1 << 14,
    K_CPU_STATUS_FLAG_RESUME = 1 << 16,
    K_CPU_STATUS_FLAG_V86_MODE = 1 << 17,
    K_CPU_STATUS_FLAG_ALIGN_CHECK = 1 << 18,
    K_CPU_STATUS_FLAG_VINTERRUPT = 1 << 19,
    K_CPU_STATUS_FLAG_VINT_PENDING = 1 << 20,
    K_CPU_STATUS_FLAG_ID = 1 << 21,
};

#define K_CPU_STATUS_REG_INIT_VALUE 0x00000002



enum K_CPU_SEG_DESC_FLAGS
{
    K_CPU_SEG_DESC_FLAG_CODE_DATA = 1 << 12,
    K_CPU_SEG_DESC_FLAG_PRESENT = 1 << 15,
    K_CPU_SEG_DESC_FLAG_64_BIT = 1 << 21,
    K_CPU_SEG_DESC_FLAG_DEFAULT_OP_SIZE = 1 << 22,
    K_CPU_SEG_DESC_FLAG_GRANULARITY = 1 << 23,
};


enum K_CPU_SEG_FLAGS
{
    /* accessed */
    K_CPU_SEG_FLAG_A = 1 << 8,
    /* code segment when set, data segment otherwise */
    K_CPU_SEG_FLAG_C = 1 << 11
};

enum K_CPU_DSEG_FLAGS
{
    /* read/write segment when set, read-only otherwise */
    K_CPU_DSEG_FLAG_RW = 1 << 9,
    /* expand down segment */
    K_CPU_DSEG_FLAG_E = 1 << 10,
};

enum K_CPU_CSEG_FLAGS
{
    /* execute/read segment when set, execute-only otherwise */
    K_CPU_CSEG_FLAG_ER = 1 << 9,
    /* conforming segment (not to be confused with K_MEM_SEG_FLAG_C, 
        which tells whether the segment is a code or data segment) */
    K_CPU_CSEG_FLAG_C = 1 << 10
};

enum K_CPU_SYS_SEG_TYPES
{
    K_CPU_SYS_SEG_TYPE_TSS16_AVAL = 1 << 8,
    K_CPU_SYS_SEG_TYPE_LDT = 2 << 8,
    K_CPU_SYS_SEG_TYPE_TSS16_BUSY = 3 << 8,
    K_CPU_SYS_SEG_TYPE_CG16 = 4 << 8,
    K_CPU_SYS_SEG_TYPE_TG = 5 << 8,
    K_CPU_SYS_SEG_TYPE_IG16 = 6 << 8,
    K_CPU_SYS_SEG_TYPE_TG16 = 7 << 8,
    K_CPU_SYS_SEG_TYPE_TSS32_AVAL = 9 << 8,
    K_CPU_SYS_SEG_TYPE_TSS32_BUSY = 11 << 8,
    K_CPU_SYS_SEG_TYPE_CG32 = 12 << 8,
    K_CPU_SYS_SEG_TYPE_IG32 = 14 << 8,
    K_CPU_SYS_SEG_TYPE_TG32 = 15 << 8
};
struct k_cpu_seg_desc_t
{
    uint32_t dw0;
    uint32_t dw1;
};



extern void k_cpu_EnableInterrupts();

extern void k_cpu_DisableInterrupts();

extern void k_cpu_EnablePaging();

extern void k_cpu_DisablePaging();

extern uint32_t k_cpu_IsPagingEnabled();

extern void k_cpu_InvalidateTLB(uint32_t address);

extern void k_cpu_Halt();

extern fastcall void k_cpu_OutB(uint8_t value, uint16_t port);

extern fastcall void k_cpu_OutW(uint16_t value, uint16_t port);

extern fastcall void k_cpu_OutD(uint32_t value, uint16_t port);

extern fastcall uint8_t k_cpu_InB(uint16_t port); 

extern fastcall uint16_t k_cpu_InW(uint16_t port);

extern fastcall uint32_t k_cpu_InD(uint16_t port);

extern void k_cpu_WriteMSR(uint32_t reg, uint64_t value);

extern uint64_t k_cpu_ReadMSR(uint32_t reg);

#endif