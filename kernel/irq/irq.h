#ifndef K_IRQ_H
#define K_IRQ_H

#include <stdint.h>

struct k_irq_desc_t
{
    uint32_t dw0;
    uint32_t dw1;
};

enum K_IRQ_DESC_TYPES
{
    K_IRQ_DESC_TYPE_INT_GATE = 6 << 8,
    K_IRQ_DESC_TYPE_TRAP_GATE = 7 << 8
};

#define K_IRQ_DESCRIPTOR(offset, segment, dpl, type, flags) \
 (struct k_irq_desc_t){ \
     .dw0 = ((uint32_t)(offset) & 0x0000ffff) | ((uint32_t)(segment) << 16), \
     .dw1 = ((uint32_t)(offset) & 0xffff0000) | (type) | K_IRQ_DESC_FLAG_PRESENT | (flags) | (K_CPU_SEG_DESC_DPL(dpl)) }

enum K_IRQ_DESC_FLAGS
{
    K_IRQ_DESC_FLAG_PRESENT = 1 << 15,
    K_IRQ_DESC_FLAG_32BIT = 1 << 11
};

enum K_IRQ_HANDLERS
{
    /* devide error */
    K_IRQ_HANDLER_DE = 0,
    /* debug */
    K_IRQ_HANDLER_DB = 1,
    /* non-maskable interrupt */
    K_IRQ_HANDLER_NMI = 2,
    /* breakpoint */
    K_IRQ_HANDLER_BP = 3,
    /* integer overflow */
    K_IRQ_HANDLER_OF = 4,
    /* bound range exceeded */
    K_IRQ_HANDLER_BR = 5,
    /* invalid opcode */
    K_IRQ_HANDLER_UD = 6,
    /* device not available */
    K_IRQ_HANDLER_NM = 7,
    /* double fault */
    K_IRQ_HANDLER_DF = 8,

    K_IRQ_HANDLER_BAD_TSS = 10,
    /* general protection */
    K_IRQ_HANDLER_GP = 13,
    /* page fault */
    K_IRQ_HANDLER_PF = 14,

    // K_INT_HANDLER_CMCI = 32,
    // K_INT_HANDLER_THERM = 33,
    // K_INT_HANDLER_LINT0 = 34,
    // K_INT_HANDLER_LINT1 = 35,
    // K_INT_HANDLER_ERROR = 36,
    // K_INT_HANDLER_TIMOUT = 37,
    // K_INT_HANDLER_TIME_SLICE = 38,
    K_IRQ_HANDLER_LAST = 256
};

typedef void (k_irq_handler_func_t)(uint32_t irq_vector, void *data);

struct k_irq_handler_t
{
    k_irq_handler_func_t *  handler;
    void *                  data;
};

enum K_IRQ_PF_FLAGS
{
    K_IRQ_PF_FLAG_NON_PAGED = 1,
    K_IRQ_PF_FLAG_WRITE = 1 << 1,
    K_IRQ_PF_FLAG_USER = 1 << 2,
    K_IRQ_PF_FLAG_RES = 1 << 3,
    K_IRQ_PF_FLAG_INSTR_FETCH = 1 << 4,
};

enum K_IRQ_INT_ERROR_CODE_FLAGS
{
    K_IRQ_ERROR_CODE_FLAG_EXT = 1,
    K_IRQ_ERROR_CODE_FLAG_DESC_LOC = 1 << 1,
    K_IRQ_ERROR_CODE_FLAG_LDT = 1 << 2,
};

void k_int_Init();

void k_irq_SetIDTEntry(uint32_t vector, uintptr_t handler, uint32_t seg_sel, uint32_t gate_pl);

void k_irq_SetInterruptHandler(uint32_t vector, k_irq_handler_func_t *handler, void *data);

void k_int_Int0(uint32_t eip, uint16_t cs);

void k_int_Int1();

void k_int_Int3(uint32_t eip, uint16_t cs);

void k_int_Int4(uint32_t eip, uint16_t cs);

void k_int_Int5(uint32_t eip, uint16_t cs);

void k_int_Int6(uint32_t eip, uint16_t cs);

void k_int_Int7(uint32_t eip, uint16_t cs);

void k_int_Int8();

void k_int_Int10(uint32_t error, uint32_t eip, uint32_t cs);

void k_int_Int13(uint32_t error, uint32_t eip, uint32_t cs);

void k_int_Int14(uint32_t address, uint32_t error, uint32_t eip, uint32_t cs);

void k_int_Intn();

void k_int_Int32();

void k_int_Int33();

void k_int_Int34();

void k_int_Int35();

void k_int_Int36();

void k_int_Int69();

void k_irq_DispatchIRQ(uint32_t vector);


#endif