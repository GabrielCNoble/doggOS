#ifndef K_CPU_DEFS_H
#define K_CPU_DEFS_H

#include <stdint.h>

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

enum K_CPU_SEG_DESC_TYPE
{
    K_CPU_SEG_DESC_TYPE_SYSTEM = 0,
    K_CPU_SEG_DESC_TYPE_CODE_DATA = 1 << 12,
};

/* 4th word */
enum K_CPU_SEG_GRAN
{
    K_CPU_SEG_GRAN_BYTE = 0,
    K_CPU_SEG_GRAN_4KB = 1 << 7
};

/* 4th word, also known as D/B flag */
enum K_CPU_SEG_OP_SIZE
{
    K_CPU_SEG_OP_SIZE_16 = 0,
    K_CPU_SEG_OP_SIZE_32 = 1 << 6,
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

/* 3rd word */
enum K_CPU_DSEG_TYPES
{
    K_CPU_DSEG_TYPE_RO = K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_ROA = K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_RW = K_CPU_DSEG_FLAG_RW | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_RWA = K_CPU_DSEG_TYPE_RW | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,

    K_CPU_DSEG_TYPE_ROE = K_CPU_DSEG_TYPE_RO | K_CPU_DSEG_FLAG_E | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_ROEA = K_CPU_DSEG_TYPE_ROE | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_RWE = K_CPU_DSEG_TYPE_RW | K_CPU_DSEG_FLAG_E | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_DSEG_TYPE_RWEA = K_CPU_DSEG_TYPE_RWE | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA
};

enum K_CPU_CSEG_FLAGS
{
    /* execute/read segment when set, execute-only otherwise */
    K_CPU_CSEG_FLAG_ER = 1 << 9,
    /* conforming segment (not to be confused with K_MEM_SEG_FLAG_C, 
        which tells whether the segment is a code or data segment) */
    K_CPU_CSEG_FLAG_C = 1 << 10
};

/* 3rd word */
enum K_CPU_CSEG_TYPES
{
    K_CPU_CSEG_TYPE_EO = K_CPU_SEG_FLAG_C | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_EOA = K_CPU_CSEG_TYPE_EO | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_ER = K_CPU_SEG_FLAG_C | K_CPU_CSEG_FLAG_ER | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_ERA = K_CPU_CSEG_TYPE_ER | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,

    K_CPU_CSEG_TYPE_EOC = K_CPU_CSEG_TYPE_EO | K_CPU_CSEG_FLAG_C | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_EOCA = K_CPU_CSEG_TYPE_EOC | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_ERC = K_CPU_CSEG_TYPE_ER | K_CPU_CSEG_FLAG_C | K_CPU_SEG_DESC_TYPE_CODE_DATA,
    K_CPU_CSEG_TYPE_ERCA = K_CPU_CSEG_TYPE_ERC | K_CPU_SEG_FLAG_A | K_CPU_SEG_DESC_TYPE_CODE_DATA
};


enum K_CPU_TSEG_FLAGS
{
    K_CPU_TSEG_FLAG_BUSY = 1 << 9
};

/* 3rd word */
enum K_CPU_SSEG_TYPES
{
    // K_CPU_SSEG_TYPE_TSS16_AVAL = K_CPU_SEG_DESC_TYPE_SYSTEM | 1 << 8,
    K_CPU_SSEG_TYPE_LDT = K_CPU_SEG_DESC_TYPE_SYSTEM | (2 << 8),
    // K_CPU_SSEG_TYPE_TSS16_BUSY = K_CPU_SEG_DESC_TYPE_SYSTEM | 3 << 8,
    // K_CPU_SSEG_TYPE_CG16 = K_CPU_SEG_DESC_TYPE_SYSTEM | 4 << 8,
    // K_CPU_SSEG_TYPE_TG = K_CPU_SEG_DESC_TYPE_SYSTEM | 5 << 8,
    // K_CPU_SSEG_TYPE_IG16 = K_CPU_SEG_DESC_TYPE_SYSTEM | 6 << 8,
    // K_CPU_SSEG_TYPE_TG16 = K_CPU_SEG_DESC_TYPE_SYSTEM | 7 << 8,
    K_CPU_SSEG_TYPE_TSS32_AVAL = K_CPU_SEG_DESC_TYPE_SYSTEM | (9 << 8),
    K_CPU_SSEG_TYPE_TSS32_BUSY = K_CPU_SSEG_TYPE_TSS32_AVAL | K_CPU_TSEG_FLAG_BUSY,
    // K_CPU_SSEG_TYPE_CG32 = K_CPU_SEG_DESC_TYPE_SYSTEM | 12 << 8,
    // K_CPU_SSEG_TYPE_IG32 = K_CPU_SEG_DESC_TYPE_SYSTEM | 14 << 8,
    // K_CPU_SSEG_TYPE_TG32 = K_CPU_SEG_DESC_TYPE_SYSTEM | 15 << 8
};

struct k_cpu_seg_desc_t
{
    union
    {
        /* make sure the compiler always aligns this struct to 8 byte boundaries */
        uint64_t qw;

        struct
        {
            uint16_t w0;
            uint16_t w1;
            uint16_t w2;
            uint16_t w3;
        } w;

    } v;
};

#define K_CPU_SEG_DESC_DPL(level) ((level) << 13)

#define K_CPU_SEG_DESC(base, limit, seg_type, dpl, gran, op_size, present) \
    ((struct k_cpu_seg_desc_t){ \
        .v.w.w0 = (uint16_t)(limit), \
        .v.w.w1 = (uint16_t)(base), \
        .v.w.w2 = (uint16_t)((((uint32_t)(base)) >> 16) & 0xff) | (uint16_t)(seg_type) | (K_CPU_SEG_DESC_DPL(dpl)) | (uint16_t)((present) << 15), \
        .v.w.w3 = (uint16_t)((((uint32_t)(limit)) >> 16) & 0xf) | (uint16_t)(op_size) | (uint16_t)(gran) | (uint16_t)((((uint32_t)(base)) >> 16) & 0xff00)})

enum K_CPU_SEG_SEL_FLAGS
{
    K_CPU_SEG_SEL_FLAG_TI = 1 << 2
};

#define K_CPU_SEG_SEL(index, rpl, ti) ((index << 3) | (ti) | (rpl & 0x3))

struct k_cpu_tss_t 
{
    uint16_t prev_link; // 0
    uint16_t res0;      // 2

    uint32_t esp0;      // 4
    uint16_t ss0;       // 8
    uint16_t res1;      // 10

    uint32_t esp1;      // 12
    uint16_t ss1;       // 16
    uint16_t res2;      // 18

    uint32_t esp2;      // 20
    uint16_t ss2;       // 24
    uint16_t res3;      // 26

    uint32_t cr3;       // 28
    uint32_t eip;       // 32  
    uint32_t eflags;    // 36
    uint32_t eax;       // 40
    uint32_t ecx;       // 44
    uint32_t edx;       // 48
    uint32_t ebx;       // 52
    uint32_t esp;       // 56
    uint32_t ebp;       // 60
    uint32_t esi;       // 64  
    uint32_t edi;       // 68

    uint16_t es;        // 72
    uint16_t res4;      // 74

    uint16_t cs;        // 76
    uint16_t res5;      // 78

    uint16_t ss;        // 80
    uint16_t res6;      // 82

    uint16_t ds;        // 84
    uint16_t res7;      // 86

    uint16_t fs;        // 88
    uint16_t res8;      // 90 

    uint16_t gs;        // 92
    uint16_t res9;      // 94

    uint16_t ldt_sel;   // 96
    uint16_t res10;     // 98

    uint16_t t_res11;   // 100
    uint16_t io_map;    // 102

    uint32_t ssp;       // 104
};

struct k_cpu_core_state_t
{
    
};

#endif