#include "proc.h"
#include "../../proc/proc.h"
#include "../../proc/thread.h"

void k_sys_TerminateProcess(struct k_sys_syscall_args_t *args)
{
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    k_proc_TerminateProcess(args->args[1]);
}

uint32_t k_sys_WaitProcess(struct k_sys_syscall_args_t *args)
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    struct k_proc_thread_t *wait_thread = (struct k_proc_thread_t *)args->args[1];
    uintptr_t *return_address = (uintptr_t *)(args->args[2] + current_thread->kernel_stack_offset);
    k_proc_WaitThread(wait_thread, return_address);
    return K_STATUS_OK;
}
