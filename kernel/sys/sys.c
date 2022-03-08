#include "sys.h"
#include "term.h"
#include "../k_int.h"
#include "../cpu/k_cpu.h"
#include "../rt/alloc.h"
#include "defs.h"
#include "syscall.h"
#include "../proc/defs.h"
#include <stdint.h>

extern void *k_sys_SysCall_a;
// extern uint16_t *k_sys_term_buffer;

void k_sys_Init()
{
    k_int_SetInterruptHandler(K_SYS_SYSCALL_VECTOR, (uintptr_t)&k_sys_SysCall_a, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0), 3);
    k_sys_TerminalInit();
}