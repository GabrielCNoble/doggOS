#include "irq.h"
#include "../sys/sys.h"
#include "../sys/term.h"
#include "../rt/mem.h"
#include "../proc/defs.h"
// #include "../proc/proc.h"
// #include "../sys/syscall.h"
#include "../cpu/k_cpu.h"
// #include "../dev/pci/piix3/isa.h"
#include "../k_rng.h"
#include "apic.h"

#include <stdarg.h>


extern void *k_int0_a;
extern void *k_int2_a;
extern void *k_int3_a;
extern void *k_int4_a;
extern void *k_int5_a;
extern void *k_int6_a;
extern void *k_int7_a;
extern void *k_int8_temp_a;
extern void *k_int8_a;
extern void *k_int10_a;
extern void *k_int13_a;
extern void *k_int14_a;
extern void *k_irq_IrqJumpTable_a;

extern void *k_proc_PreemptThread_a;
extern void *k_proc_StartUserThread_a;
extern void *k_sys_SysCall_a;

struct k_irq_desc_t     k_irq_idt[K_IRQ_HANDLER_LAST];
struct k_irq_handler_t  k_irq_handler_table[K_IRQ_HANDLER_LAST];

void k_int_Init()
{
    k_irq_idt[K_IRQ_HANDLER_DE] = K_IRQ_DESCRIPTOR(&k_int0_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_NMI] = K_IRQ_DESCRIPTOR(&k_int2_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_BP] = K_IRQ_DESCRIPTOR(&k_int3_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_TRAP_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_OF] = K_IRQ_DESCRIPTOR(&k_int4_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_TRAP_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_BR] = K_IRQ_DESCRIPTOR(&k_int5_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_UD] = K_IRQ_DESCRIPTOR(&k_int6_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_NM] = K_IRQ_DESCRIPTOR(&k_int7_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_DF] = K_IRQ_DESCRIPTOR(&k_int8_temp_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_BAD_TSS] = K_IRQ_DESCRIPTOR(&k_int10_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_GP] = K_IRQ_DESCRIPTOR(&k_int13_a, K_CPU_SEG_SEL(6, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
    k_irq_idt[K_IRQ_HANDLER_PF] = K_IRQ_DESCRIPTOR(&k_int14_a, K_CPU_SEG_SEL(2, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);

    for(uint32_t index = 32; index < K_IRQ_HANDLER_LAST; index++)
    {
        uintptr_t offset = ((uintptr_t)&k_irq_IrqJumpTable_a) + (index - 32) * 16;
        k_irq_idt[index] = K_IRQ_DESCRIPTOR(offset, K_CPU_SEG_SEL(K_PROC_R0_C_CODE_SEG, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
        k_irq_handler_table[index].handler = NULL;
        k_irq_handler_table[index].data = NULL;
    }
    
    k_cpu_Lidt(k_irq_idt, K_IRQ_HANDLER_LAST);
}

void k_irq_SetIDTEntry(uint32_t vector, uintptr_t handler, uint32_t seg_sel, uint32_t gate_pl)
{
    k_irq_idt[vector] = K_IRQ_DESCRIPTOR(handler, seg_sel, gate_pl, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);
}

void k_irq_SetInterruptHandler(uint32_t vector, k_irq_handler_func_t *handler, void *data)
{
    if(vector >= 32 && vector < K_IRQ_HANDLER_LAST)
    {   
        uintptr_t offset = ((uintptr_t)&k_irq_IrqJumpTable_a) + (vector - 32) * 16;
        struct k_irq_desc_t desc = K_IRQ_DESCRIPTOR(offset, K_CPU_SEG_SEL(K_PROC_R0_C_CODE_SEG, 3, 0), 3, K_IRQ_DESC_TYPE_INT_GATE, K_IRQ_DESC_FLAG_32BIT);

        if(k_irq_idt[vector].dw0 == desc.dw0 && k_irq_idt[vector].dw1 == desc.dw1)
        {
            /* entries directly put in the idt take priority over other installed irq handlers */
            k_irq_handler_table[vector].handler = handler;
            k_irq_handler_table[vector].data = data;
        }
        else
        {
            k_sys_TerminalPrintf("interrupt handler for irq %d not installed!\n", vector);
        }
    }
}

void k_int_Int0(uint32_t eip, uint16_t cs)
{
    // k_sys_TerminalPrintf("division by zero at %x:%x\n", (uint32_t)cs, eip);
    k_sys_HaltAndCatchFire(K_EXCEPTION_DIVISION_BY_ZERO, eip, cs);
}

void k_int_Int2()
{
    k_sys_TerminalPrintf("NMI!\n");
}

void k_int_Int3(uint32_t eip, uint16_t cs)
{
    k_sys_TerminalPrintf("breakpoint! next instruction at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int4(uint32_t eip, uint16_t cs)
{
    k_sys_TerminalPrintf("overflow! next instruction at %x: %x\n", (uint32_t)cs, eip);
}

void k_int_Int5(uint32_t eip, uint16_t cs)
{
    k_sys_TerminalPrintf("int 5\n");
}

void k_int_Int6(uint32_t eip, uint16_t cs)
{
    // k_sys_TerminalPrintf("invalid instruction at %x:%x\n", (uint32_t)cs, eip);
    k_sys_HaltAndCatchFire(K_EXCEPTION_INVALID_OPCODE, eip, (uint32_t)cs);
}

void k_int_Int7(uint32_t eip, uint16_t cs)
{
    // k_sys_TerminalPrintf("device not available at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int8()
{
    k_sys_TerminalPrintf("double fault!\n");
}

void k_int_Int10(uint32_t error, uint32_t eip, uint32_t cs)
{
    k_sys_TerminalPrintf("invalid tss exception at %x:%x, with code %x\n", cs, eip, error);
}

void k_int_Int13(uint32_t error, uint32_t eip, uint32_t cs)
{
    // k_sys_TerminalPrintf("general protection fault at %x:%x, with error %x\n", (uint32_t)cs, eip, error);
    k_sys_HaltAndCatchFire(K_EXCEPTION_GENERAL_PROTECTION_FAULT, eip, cs, error);
}

void k_int_Int14(uint32_t address, uint32_t error, uint32_t eip, uint32_t cs)
{
    // k_sys_TerminalPrintf("page fault at %x:%x, with error %x\n", cs, eip, error);
    // k_sys_TerminalPrintf(k_int_pf_messages[error], address);

    k_sys_HaltAndCatchFire(K_EXCEPTION_PAGE_FAULT, eip, cs, address, error);
}

// void k_int_Intn()
// {
//     k_sys_TerminalPrintf("this exception has not been implemented!\n");
// }

// void k_int_Int32()
// {
//     // k_printf("cmci\n");
//     // k_cpu_InB(0x60);
//     // k_PIIX3_82C59_EndOfInterrupt();
//     // k_sys_TerminalPrintf("cock\n");
//     k_PIIX3_ISA_EndOfInterrupt(0);
// }


// void k_int_Int33()
// {
//     // k_PIIX3_82C59_EndOfInterrupt();
//     // k_sys_TerminalPrintf("cock\n");
// }

// void k_int_Int34()
// {
//     k_sys_TerminalPrintf("lint0\n");
// }

// void k_int_Int35()
// {
//     k_sys_TerminalPrintf("lint1\n");
// }

// void k_int_Int36()
// {
//     k_sys_TerminalPrintf("error\n"); 
// }

void k_int_HaltAndCatchFire2()
{
    // k_int_HaltAndCatchFire2();

    k_cpu_DisableInterrupts();
    // k_term_clear();
    k_sys_TerminalClear();
    k_sys_TerminalPrintf("A terrible TERRIBLE thing has happened.\n");
    // k_printf("Setting stuff on fire. Please hold...\n");
    k_sys_TerminalPrintf("The machine will now halt and catch fire.\n");

    uint8_t fire_buffer[80 * 25] = {};
    uint8_t rand_offset_cursor = 0;
    // uint8_t rand_offsets[] = { 2, 13, 46, 4, 9, 37, 52, 14, 9, 7, 13, 0, 5, 128, 37, 1, 63, 6, 255, 42, 51, 8, 97, 3, 6, 97, 65};
    // uint8_t rand_offsets[] = {23, 27, 6, 3, 22, 8, 27, 21, 34, 30, 6, 11, 18, 27, 30, 23, 15, 32};
    // uint8_t rand_offsets[] = {0, 3, 1, 2, 3, 0, 2, 0, 3, 1, 3, 2, 0, 1, 2, 3};
    uint8_t rand_offsets[] = {0, 3, 1, 2, 3, 0, 2, 0, 3, 1, 3, 2, 0, 1, 2, 3, 0, 3, 2, 1, 1, 2, 0, 3, 0, 1};

    // uint16_t colors[] = 
    // {
    //     k_vga_char(178, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BLACK)),
    //     k_vga_char(178, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BROWN)),
    //     k_vga_char(177, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BROWN)),
    //     k_vga_char(177, k_vga_attrib(K_VGA_COLOR_GREEN, K_VGA_COLOR_LIGHT_RED)),
    //     k_vga_char(177, k_vga_attrib(K_VGA_COLOR_LIGHT_BROWN, K_VGA_COLOR_LIGHT_RED)),
    //     k_vga_char(219, k_vga_attrib(K_VGA_COLOR_LIGHT_BROWN, K_VGA_COLOR_LIGHT_BROWN)),
    //     k_vga_char(177, k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_LIGHT_BROWN)),
    //     k_vga_char(219, k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_WHITE)),
    // };

    // for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    // {
    //     for(uint32_t row_index = 1; row_index < k_gfx_vga_height; row_index++)
    //     {
    //         fire_buffer[row_index * k_gfx_vga_width + column_index] = 0;
    //     }
    // }

    // for(uint32_t delay = 0; delay < 0x2fffffff; delay++);

    // for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    // {
    //     fire_buffer[k_gfx_vga_width * (k_gfx_vga_height - 1) + column_index] = 7;
    // }

    // while(1)
    // {
    //     for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    //     {
    //         for(uint32_t row_index = 3; row_index < k_gfx_vga_height; row_index++)
    //         {
    //             uint32_t from = row_index * k_gfx_vga_width + column_index;

    //             if(fire_buffer[from] > 0)
    //             {
    //                 uint32_t rand = k_rng_Rand() % 4;
    //                 uint32_t to = from - k_gfx_vga_width - (rand % 3);
    //                 fire_buffer[to] = fire_buffer[from] - (rand & 1);
    //             }

    //             k_gfx_vga_cur_mem_map[k_gfx_vga_width * row_index + column_index] = colors[fire_buffer[k_gfx_vga_width * row_index + column_index]];
    //         }
    //     }

    //     for(uint32_t delay = 0; delay < 0xffffff; delay++);
    // }

    k_cpu_Halt();
}

void k_irq_DispatchIRQ(uint32_t vector)
{
    struct k_irq_handler_t *handler = k_irq_handler_table + vector;
    if(handler->handler != NULL)
    {
        
        handler->handler(vector, handler->data);
    }
}