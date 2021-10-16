#include "dg_malloc.h"
#include "../../kernel/mem/k_alloc.h"
#include "../../kernel/proc/k_proc.h"

void *dg_Malloc(uint32_t size, uint32_t align)
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    return k_mem_Malloc(&current_thread->heap, size, align);
}

void dg_Free(void *memory)
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    k_mem_Free(&current_thread->heap, memory);
}