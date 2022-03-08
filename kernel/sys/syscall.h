#ifndef K_SYSCALL_H
#define K_SYSCALL_H

#include "defs.h"
#include "../defs.h"

enum K_SYS_SYSCALLS
{
    K_SYS_SYSCALL_CREATE_THREAD,
    K_SYS_SYSCALL_GET_CUR_THREAD,
    K_SYS_SYSCALL_WAIT_THREAD,
    K_SYS_SYSCALL_YIELD_THREAD,
    K_SYS_SYSCALL_SUSPEND_THREAD,
    K_SYS_SYSCALL_KILL_THREAD,
    K_SYS_SYSCALL_TERMINATE_THREAD,
    K_SYS_SYSCALL_LAST
};

uint32_t k_sys_SysCall(uint32_t number, ...);

uint32_t k_sys_DispatchSysCall(struct k_sys_syscall_args_t *args);

#endif
