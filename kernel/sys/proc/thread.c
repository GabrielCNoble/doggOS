#include "thread.h"
#include "../../proc/thread.h"

uint32_t k_sys_TerminateThread(struct k_sys_syscall_args_t *args)
{
    k_proc_TerminateThread(args->args[1]);
}