#include "mouse.h"
#include "../cpu/defs.h"
#include "../sys/term.h"
#include "soc/piix3/piix3.h"

extern void *k_mouse_MouseHandler_a;

void k_mouse_Init()
{
    // k_int_SetInterruptHandler(K_PIIX3_82C59_IRQ_BASE + K_MOUSE_IRQ_VECTOR, (uintptr_t)&k_mouse_MouseHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3);
}

void k_mouse_MouseHandler()
{
    k_sys_TerminalPrintf("fuck");
    k_cpu_InB(0x60);
}