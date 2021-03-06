#ifndef K_SYSCALL_H
#define K_SYSCALL_H

#include "defs.h"
#include "../defs.h"

enum K_SYS_SYSCALLS
{
    K_SYS_SYSCALL_TEST_CALL,
    K_SYS_SYSCALL_CRASH,
    K_SYS_SYSCALL_CREATE_THREAD,
    K_SYS_SYSCALL_GET_CUR_THREAD,
    K_SYS_SYSCALL_WAIT_THREAD,
    K_SYS_SYSCALL_YIELD_THREAD,
    K_SYS_SYSCALL_SUSPEND_THREAD,
    K_SYS_SYSCALL_KILL_THREAD,
    K_SYS_SYSCALL_TERMINATE_THREAD,
    K_SYS_SYSCALL_TERMINATE_PROCESS,
    K_SYS_SYSCALL_WAIT_PROCESS,
    K_SYS_SYSCALL_PUTS,
    K_SYS_SYSCALL_READ_LINE,
    K_SYS_SYSCALL_LAST
};

extern uint32_t k_sys_SysCall(uint32_t number, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

uint32_t k_sys_DispatchSysCall(struct k_sys_syscall_args_t *args);

// uint32_t k_sys_TestSysCall(struct k_sys_syscall_args_t *args);

#endif
