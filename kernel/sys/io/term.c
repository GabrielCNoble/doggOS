#include "term.h"
#include "../../proc/thread.h"
#include "../../proc/proc.h"
#include "../../sys/term.h"
#include "../term.h"

uint32_t k_sys_Puts(struct k_sys_syscall_args_t *args)
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    char *str = (char *)(args->args[1] + current_thread->kernel_stack_offset);
    k_sys_TerminalPuts(str);
    return 0;
}

uint32_t k_sys_ReadLine(struct k_sys_syscall_args_t *args)
{
    // struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    char *output_buffer = (char *)(args->args[1] + current_thread->kernel_stack_offset);
    k_sys_TerminalReadLine(output_buffer, args->args[2]);
    return 0;
}
