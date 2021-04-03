#ifndef K_INT_H
#define K_INT_H

#include <stdint.h>

struct k_int_desc_t
{
    uint32_t dw0;
    uint32_t dw1;
};

enum K_INT_DESC_TYPES
{
    K_INT_DESC_TYPE_IG = 6 << 8,
    K_INT_DESC_TYPE_TG = 7 << 8
};

#define K_INT_DESCRIPTOR(offset, segment, type, flags) \
 (struct k_int_desc_t){.dw0 = ((uint32_t)(offset) & 0x0000ffff) | ((segment) << 16), .dw1 = ((uint32_t)(offset) & 0xffff0000) | (type) | K_INT_DESC_FLAG_PRESENT | (flags) }

enum K_INT_DESC_FLAGS
{
    K_INT_DESC_FLAG_PRESENT = 1 << 15,
    K_INT_DESC_FLAG_32BIT = 1 << 11
};

enum K_INT_HANDLERS
{
    /* devide error */
    K_INT_HANDLER_DE = 0,
    /* debug */
    K_INT_HANDLER_DB = 1,
    /* non-maskable interrupt */
    K_INT_HANDLER_NMI = 2,
    /* breakpoint */
    K_INT_HANDLER_BP = 3,
    /* integer overflow */
    K_INT_HANDLER_OF = 4,
    /* bound range exceeded */
    K_INT_HANDLER_BR = 5,
    /* invalid opcode */
    K_INT_HANDLER_UD = 6,
    /* device not available */
    K_INT_HANDLER_NM = 7,
    /* double fault */
    K_INT_HANDLER_DF = 8,
    /* general protection */
    K_INT_HANDLER_GP = 13,
    /* page fault */
    K_INT_HANDLR_PF = 14,
};

enum K_INT_PF_FLAGS
{
    K_INT_PF_FLAG_NON_PAGED = 1,
    K_INT_PF_FLAG_WRITE = 1 << 1,
    K_INT_PF_FLAG_USER = 1 << 2,
    K_INT_PF_FLAG_RES = 1 << 3,
    K_INT_PF_FLAG_INSTR_FETCH = 1 << 4,
};

void k_int_init();

extern void k_int_lidt();

void k_int0_handler(uint32_t eip, uint16_t cs);

void k_int1_handler();

void k_int3_handler(uint32_t eip, uint16_t cs);

void k_int4_handler(uint32_t eip, uint16_t cs);

void k_int5_handler(uint32_t eip, uint16_t cs);

void k_int6_handler(uint32_t eip, uint16_t cs);

void k_int7_handler(uint32_t eip, uint16_t cs);

void k_int8_handler();

void k_int13_handler(uint32_t error, uint32_t eip, uint16_t cs);

void k_int14_handler(uint32_t address, uint32_t error, uint32_t eip, uint16_t cs);

void k_intn_handler();



#endif