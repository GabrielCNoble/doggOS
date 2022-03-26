#ifndef SYS_THREAD_H
#define SYS_THREAD_H

#include <stdint.h>
#include "../defs.h"

uint32_t k_sys_TerminateThread(struct k_sys_syscall_args_t *args);

uint32_t k_sys_YieldThread();

#endif