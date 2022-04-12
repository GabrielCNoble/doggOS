#include "sys.h"
#include "term.h"
#include "../int/irq.h"
#include "../cpu/k_cpu.h"
#include "../rt/alloc.h"
#include "defs.h"
#include "syscall.h"
#include "../proc/defs.h"
#include <stdarg.h>

extern void *k_sys_SysCall_a;
extern uint32_t k_gfx_vga_width;
extern uint32_t k_gfx_vga_height;
extern uint16_t *k_gfx_vga_cur_mem_map;
// extern uint16_t *k_sys_term_buffer;

char *k_sys_exception_codes[] = 
{
    [K_EXCEPTION_GENERAL_PROTECTION_FAULT] = "GENERAL PROTECTION FAULT",
    [K_EXCEPTION_INVALID_OPCODE] = "INVALID OPCODE EXCEPTION",
    [K_EXCEPTION_PAGE_FAULT] = "PAGE FAULT",
    [K_EXCEPTION_DIVISION_BY_ZERO] = "DIVIDE ERROR FAULT",
    [K_EXCEPTION_UNKNOWN] = "UNKNOWN EXCEPTION",
};

char *k_sys_pf_messages[] = 
{
    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE READ ATTEMPT",

    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE INSTRUCTION FETCH ATTEMPT",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE READ PAGE-LEVEL PROTECTION VIOLATION",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE INSTRUCTION FETCH PAGE-LEVEL PROTECTION VIOLATION",

    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE WRITE ATTEMPT",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (K_IRQ_PF_FLAG_WRITE) | 
        (!K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "KERNEL-MODE WRITE ATTEMPT PAGE-LEVEL PROTECTION VIOLATION",





    



    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE READ ATTEMPT",

    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE INSTRUCTION FETCH ATTEMPT",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE READ PAGE-LEVEL PROTECTION VIOLATION",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (!K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE INSTRUCTION FETCH PAGE-LEVEL PROTECTION VIOLATION",

    [
        (!K_IRQ_PF_FLAG_NON_PAGED) | 
        (K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE WRITE ATTEMPT",

    [
        (K_IRQ_PF_FLAG_NON_PAGED) | 
        (K_IRQ_PF_FLAG_WRITE) | 
        (K_IRQ_PF_FLAG_USER) | 
        (!K_IRQ_PF_FLAG_RES) | 
        (!K_IRQ_PF_FLAG_INSTR_FETCH)
    ] = "USER-MODE WRITE ATTEMPT PAGE-LEVEL PROTECTION VIOLATION",
};

// struct k_sys_shared_data_t *k_sys_shared_data;

void k_sys_Init()
{
    k_int_SetInterruptHandler(K_SYS_SYSCALL_IRQ_VECTOR, (uintptr_t)&k_sys_SysCall_a, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0), 3);
    k_sys_TerminalInit();
}

void k_sys_HaltAndCatchFire(uint32_t exception, uint32_t eip, uint32_t cs, ...)
{
    k_cpu_DisableInterrupts();
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_LIGHT_GREY, K_SYS_TERM_COLOR_BLUE);
    k_sys_TerminalClear();
    k_sys_TerminalPrintf("\n\n\n\n\n\n                                 ");
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_BLUE, K_SYS_TERM_COLOR_LIGHT_GREY);
    k_sys_TerminalPrintf("*** CRASH! ***\n\n");
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_LIGHT_GREY, K_SYS_TERM_COLOR_BLUE);

    uint32_t value_count = 0;
    uint32_t error_values[8];
    // char *stop_code;

    k_sys_TerminalPrintf("      Something pretty terrible, like an exception or something, happened\n");
    k_sys_TerminalPrintf("      at 0x%x:0x%x. Probably your fault. Keep your grubby fingers to\n", cs, eip);
    k_sys_TerminalPrintf("      yourself next time.\n");

    k_sys_TerminalPrintf("\n\n      TECHNICAL INFORMATION:\n            %s (0x%x)\n", k_sys_exception_codes[exception], exception);

    va_list args;
    va_start(args, cs);
    
    switch(exception)
    {
        case K_EXCEPTION_GENERAL_PROTECTION_FAULT:
        {
            uint32_t error_code = va_arg(args, uint32_t);
            k_sys_TerminalPrintf("            ERROR CODE: 0x%x\n", error_code);
        }
        break;

        case K_EXCEPTION_PAGE_FAULT:
        {
            uint32_t fault_address = va_arg(args, uint32_t);
            uint32_t error_code = va_arg(args, uint32_t);
            k_sys_TerminalPrintf("            FAULT ADDRESS: 0x%x\n", fault_address);
            k_sys_TerminalPrintf("            ERROR CODE: %s (0x%x)\n", k_sys_pf_messages[error_code], error_code);
        }
        break;

        case K_EXCEPTION_DIVISION_BY_ZERO:

        break;

        case K_EXCEPTION_INVALID_OPCODE:

        break;

        case K_EXCEPTION_UNKNOWN:

        break;
    }

    va_end(args);

    // k_sys_TerminalPrintf("\n\n      TECHNICAL INFORMATION:\n            %s (0x%x)", k_sys_exception_codes[exception], exception);

    // if(value_count)
    // {
    //     k_sys_TerminalPrintf(" - ");
    //     for(uint32_t value_index = 0; value_index < value_count; value_index++)
    //     {
    //         k_sys_TerminalPrintf("0x%x ", error_values[value_index]);
    //     }
    // }
    // k_sys_TerminalPrintf("\n\n");
    // k_sys_TerminalPrintf("      The system will now halt and catch fire...\n");

    // uint8_t fire_buffer[80 * 25] = {};

    // uint16_t colors[] = 
    // {
    //     k_sys_TerminalChar(178, (K_SYS_TERM_COLOR_BLACK << 4) | K_SYS_TERM_COLOR_BLACK),
    //     k_sys_TerminalChar(178, (K_SYS_TERM_COLOR_BLACK << 4) | K_SYS_TERM_COLOR_BROWN),
    //     k_sys_TerminalChar(177, (K_SYS_TERM_COLOR_BLACK << 4) | K_SYS_TERM_COLOR_BROWN),
    //     k_sys_TerminalChar(177, (K_SYS_TERM_COLOR_GREEN << 4) | K_SYS_TERM_COLOR_LIGHT_RED),
    //     k_sys_TerminalChar(177, (K_SYS_TERM_COLOR_LIGHT_BROWN << 4) | K_SYS_TERM_COLOR_LIGHT_RED),
    //     k_sys_TerminalChar(219, (K_SYS_TERM_COLOR_LIGHT_BROWN << 4) | K_SYS_TERM_COLOR_LIGHT_BROWN),
    //     k_sys_TerminalChar(177, (K_SYS_TERM_COLOR_WHITE << 4) | K_SYS_TERM_COLOR_LIGHT_BROWN),
    //     k_sys_TerminalChar(219, (K_SYS_TERM_COLOR_WHITE << 4) | K_SYS_TERM_COLOR_WHITE),
    // };

    // for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    // {
    //     for(uint32_t row_index = 1; row_index < k_gfx_vga_height; row_index++)
    //     {
    //         fire_buffer[row_index * k_gfx_vga_width + column_index] = 0;
    //     }
    // }

    // for(uint32_t delay = 0; delay < 0x7fffffff; delay++);

    // for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    // {
    //     fire_buffer[k_gfx_vga_width * (k_gfx_vga_height - 1) + column_index] = 7;
    // }

    // uint32_t rand_var = 2;
    // uint32_t rand_timer = 0;

    // while(1)
    // {
    //     for(uint32_t column_index = 0; column_index < k_gfx_vga_width; column_index++)
    //     {
    //         for(uint32_t row_index = 0; row_index < k_gfx_vga_height; row_index++)
    //         {
    //             uint32_t from = row_index * k_gfx_vga_width + column_index;

    //             if(fire_buffer[from] > 0)
    //             {
    //                 // uint32_t v = 3 + (k_rng_Rand() % 3);
    //                 uint32_t rand = k_rng_Rand() % rand_var;
    //                 uint32_t to = from - k_gfx_vga_width - (rand % 3);
    //                 if(to < k_gfx_vga_width * k_gfx_vga_height)
    //                 {
    //                     fire_buffer[to] = fire_buffer[from] - (rand & 1);
    //                 }
    //             }

    //             uint16_t fire_color = colors[fire_buffer[k_gfx_vga_width * row_index + column_index]];
    //             uint8_t screen_color = (k_gfx_vga_cur_mem_map[k_gfx_vga_width * row_index + column_index] >> 8);

    //             if((fire_color & 0xff00) != 0 || ((screen_color >> 4) != K_SYS_TERM_COLOR_BLUE && (screen_color & 0x0f) != K_SYS_TERM_COLOR_BLUE))
    //             {
    //                 k_gfx_vga_cur_mem_map[k_gfx_vga_width * row_index + column_index] = fire_color;
    //             }

    //             rand_timer++;
    //             if(rand_timer > 19)
    //             {
    //                 rand_timer = 0;
    //                 rand_var = 3 - (k_rng_Rand() % 2);
    //             }
    //             // if((color & 0xff00) != 0 || ((color & 0xff00) == 0 && k_gfx_vga_cur_mem_map[k_gfx_vga_width * row_index + column_index]))
    //             // k_gfx_vga_cur_mem_map[k_gfx_vga_width * row_index + column_index] = colors[fire_buffer[k_gfx_vga_width * row_index + column_index]];
    //         }
    //     }

    //     for(uint32_t delay = 0; delay < 0xffffff; delay++);
    // }


    k_cpu_Halt();
}