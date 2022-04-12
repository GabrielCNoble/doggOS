#ifndef SYS_PROC_H
#define SYS_PROC_H

#include "../defs.h"

void k_sys_TerminateProcess(struct k_sys_syscall_args_t *args);

uint32_t k_sys_WaitProcess(struct k_sys_syscall_args_t *args);

#endif
