#include "dg_thread.h"
#include "../../kernel/proc/k_proc.h"

struct dg_thread_t *dg_CreateThread(uint32_t (*thread_func)(void *data))
{
    uint32_t thread_id = k_proc_CreateThread(thread_func, 3);
    struct dg_thread_t *thread = (struct dg_thread_t *)k_proc_GetThread(thread_id);
    return thread;
}

uint32_t dg_WaitThread(struct dg_thread_t *thread)
{

}