#ifndef K_SYS_DEFS_H
#define K_SYS_DEFS_H

#include <stdint.h>

#define K_SYS_SYSCALL_VECTOR 69
#define K_SYS_SYSCALL_MAX_ARGS 8

/* sizeof(uint32_t) * 8 == 32 */
struct k_sys_syscall_args_t
{
    uint32_t args[K_SYS_SYSCALL_MAX_ARGS];
};

#endif