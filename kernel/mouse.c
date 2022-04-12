#include "mouse.h"
#include "int/irq.h"
#include "cpu/k_defs.h"
#include "sys/term.h"

extern void *k_mouse_MouseHandler_a;

void k_mouse_Init()
{
    k_int_SetInterruptHandler(44, (uintptr_t)&k_mouse_MouseHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3);
}

void k_mouse_MouseHandler()
{
    k_sys_TerminalPrintf("fuck");
    k_cpu_InB(0x60);
}