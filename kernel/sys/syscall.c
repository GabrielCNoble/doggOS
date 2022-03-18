#include "syscall.h"
#include "../proc/defs.h"
#include "../cpu/k_defs.h"
#include "../int/int.h"
#include "term.h"
#include "proc/thread.h"
#include <stdarg.h>

uint32_t k_sys_TestSysCall(struct k_sys_syscall_args_t *args)
{
    k_sys_TerminalPrintf("k_sys_TestSysCall: args have values: %x %x %x\n", args->args[1], args->args[2], args->args[3]);
    return 0xfffff3ea;
}

uint32_t k_sys_CrashSysCall(struct k_sys_syscall_args_t *args)
{
    uintptr_t *crash = (uint32_t *)0x1234567;
    *crash = 0xb00b1e5;
}

uint32_t (*k_sys_syscall_table[])(struct k_sys_syscall_args_t *args) = {
    [K_SYS_SYSCALL_TEST_CALL] = k_sys_TestSysCall,
    [K_SYS_SYSCALL_CRASH] = k_sys_CrashSysCall,
};

uint32_t k_sys_DispatchSysCall(struct k_sys_syscall_args_t *args)
{
    uint32_t status = K_STATUS_OK;

    k_cpu_EnableInterrupts();

    if(args->args[0] < K_SYS_SYSCALL_LAST)
    {
        status = k_sys_syscall_table[args->args[0]](args);
    }

    return status;
}