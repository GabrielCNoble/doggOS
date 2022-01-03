#include "dg_thread.h"
#include "../../kernel/proc/k_thread.h"

struct dg_thread_t *dg_CreateThread(uint32_t (*thread_func)(void *data))
{
    struct k_proc_thread_t *thread = k_proc_CreateThread(thread_func, NULL);
    // struct dg_thread_t *thread = (struct dg_thread_t *)k_proc_GetThread(thread_id);
    return (struct dg_thread_t *)thread;
}

uint32_t dg_WaitThread(struct dg_thread_t *thread)
{
    (void)thread;
    return 0;
}