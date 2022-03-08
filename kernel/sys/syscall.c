#include "syscall.h"
#include "../proc/defs.h"
#include "../cpu/k_defs.h"
#include "../k_int.h"
#include "proc/thread.h"
#include <stdarg.h>

uint32_t (*k_sys_syscall_table[])(struct k_sys_syscall_args_t *args) = {
    // [K_SYS_SYSCALL_TERMINATE_THREAD] = k_sys_TerminateThread
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