#ifndef SYS_TERM_H
#define SYS_TERM_H

#include "../defs.h"

uint32_t k_sys_Puts(struct k_sys_syscall_args_t *args);

uint32_t k_sys_ReadLine(struct k_sys_syscall_args_t *args);

#endif
