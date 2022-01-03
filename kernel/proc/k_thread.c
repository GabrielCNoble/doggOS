#include "k_thread.h"
#include "k_proc.h"
#include "../mem/k_objlist.h"
#include "../mem/k_alloc.h"
#include "../timer/k_apic.h"

extern struct k_proc_thread_t k_proc_scheduler_thread;
extern struct k_mem_objlist_t k_proc_threads;
extern struct k_proc_thread_t *k_proc_current_thread;
extern struct k_proc_thread_t *k_proc_finished_threads;

struct k_proc_thread_t *k_proc_CreateThread(uintptr_t (*thread_fn)(void *data), void *data)
{
    uint32_t thread_id = k_mem_AllocObjListElement(&k_proc_threads);
    struct k_proc_thread_t *thread = k_mem_GetObjListElement(&k_proc_threads, thread_id);
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();

    uint32_t ring = current_process->ring;

    thread->tid = thread_id;
    thread->process = current_process;

    for(uint32_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        thread->heap.buckets[bucket_index].first_chunk = NULL;
        thread->heap.buckets[bucket_index].last_chunk = NULL;
    }

    thread->heap.free_pages = NULL;
    thread->heap.big_heap = &current_process->heap;

    if(!current_process->threads)
    {
        current_process->threads = thread;
    }
    else
    {
        current_process->last_thread->next = thread;
    }

    thread->prev = current_process->last_thread;
    current_process->last_thread = thread;

    /* thread stack gets allocated from the process heap */
    uintptr_t stack = (uintptr_t)k_mem_Malloc(&current_thread->heap, K_PROC_THREAD_WORK_STACK_SIZE, 4);
    thread->stack_base = stack;
    thread->entry_point = thread_fn;

    stack += K_PROC_THREAD_WORK_STACK_SIZE;

    thread->start_esp = (uintptr_t *)stack;
    thread->current_esp = thread->start_esp;
    thread->state = K_PROC_THREAD_STATE_READY;
    thread->page_dir = current_process->page_map.pdir_page;

    uintptr_t start_ebp = (uintptr_t)thread->start_esp;
    uint32_t cs = K_CPU_SEG_SEL(2, 0, 0);
    uint32_t ss = K_CPU_SEG_SEL(1, 0, 0);
    uint32_t ds = K_CPU_SEG_SEL(1, 0, 0);

    if(ring)
    {
        /* threads not in ring 0 will have a dedicated "stack" to store its context,
        while threads in ring 0 will store its context at the very end of their work stack */
        uintptr_t state_block = (uintptr_t)k_mem_Malloc(&k_proc_scheduler_thread.heap, K_PROC_THREAD_KERNEL_STACK_SIZE, 4);
        state_block += K_PROC_THREAD_KERNEL_STACK_SIZE;
        thread->current_esp = (uintptr_t *)state_block;

        cs = K_CPU_SEG_SEL(4, 3, 0);
        ss = K_CPU_SEG_SEL(3, 3, 0);
        ds = K_CPU_SEG_SEL(3, 3, 0);

        /* setup the stack for a call to k_proc_StartThread */
        thread->current_esp--;
        *thread->current_esp = (uintptr_t)thread;

        /* normally the return address would go here, but we won't be returning from 
        k_proc_StartThread. Instead, we'll just switch to the scheduler once the thread 
        callback returns */
        thread->current_esp--;
        *thread->current_esp = 0;

        /* ss */
        thread->current_esp--;
        *thread->current_esp = ss;
        /* esp */
        thread->current_esp--;
        *thread->current_esp = (uintptr_t )thread->start_esp;

        thread->start_esp = (uintptr_t *)state_block;
    }
    else
    {
        /* setup the stack for a call to k_proc_StartThread */
        thread->current_esp--;
        *thread->current_esp = (uintptr_t)thread;

        /* normally the return address would go here, but we won't be returning from 
        k_proc_StartThread. Instead, we'll just switch to the scheduler once the thread 
        callback returns */
        thread->current_esp--;
        *thread->current_esp = 0;
    }

    thread->return_data = (uintptr_t)data;

    /* eflags */
    thread->current_esp--;
    *thread->current_esp = K_CPU_STATUS_REG_INIT_VALUE | K_CPU_STATUS_FLAG_INT_ENABLE;
    /* cs */
    thread->current_esp--;
    *thread->current_esp = cs;
    /* eip */
    thread->current_esp--;
    *thread->current_esp = (uintptr_t)k_proc_RunThreadCallback;
    // *thread->current_esp = thread->entry_point;
    /* eax */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* ebx */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* ecx */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* edx */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* esi */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* edi */
    thread->current_esp--;
    *thread->current_esp = 0;
    /* ebp */
    thread->current_esp--;
    *thread->current_esp = start_ebp;
    /* ds */
    thread->current_esp--;
    *thread->current_esp = ds;
    /* es */
    thread->current_esp--;
    *thread->current_esp = ds;
    /* fs */
    thread->current_esp--;
    *thread->current_esp = ds;

    return thread;
}

struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id)
{
    struct k_proc_thread_t *thread = NULL;
    thread = k_mem_GetObjListElement(&k_proc_threads, thread_id);

    if(thread && thread->tid == K_PROC_INVALID_THREAD_ID)
    {
        thread = NULL;
    }

    return thread;
}

struct k_proc_thread_t *k_proc_GetCurrentThread()
{
    return k_proc_current_thread;
}

void k_proc_DestroyThread(struct k_proc_thread_t *thread)
{
    if(thread && thread->tid != K_PROC_INVALID_THREAD_ID)
    {
        struct k_proc_process_t *current_process = thread->process;

        if(current_process->ring)
        {
            /* the two topmost positions in the stack are occupied by the stack frame
            to the initial call to k_proc_RunThreadCallback. The third position contains
            the stack segment selector, and the fourth contains the pointer to the actual
            user stack. */
            void *user_stack = (void *)(thread->start_esp[-4] - K_PROC_THREAD_WORK_STACK_SIZE);
            k_mem_Free(&thread->heap, user_stack);
        }

        void *kernel_stack = (void *)((uintptr_t)thread->start_esp - K_PROC_THREAD_KERNEL_STACK_SIZE);
        k_mem_Free(&k_proc_scheduler_thread.heap, kernel_stack);
        k_mem_FreeObjListElement(&k_proc_threads, thread->tid);
        thread->tid = K_PROC_INVALID_THREAD_ID;
    }
}

void k_proc_KillThread(uint32_t thread_id)
{
    struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);

    if(thread)
    {

    }
}

void k_proc_SuspendThread(uint32_t thread_id)
{
    (void)thread_id;
}

uint32_t k_proc_WaitThread(uint32_t thread_id)
{
    struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);      
}

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}

void k_proc_RunThreadCallback(struct k_proc_thread_t *thread)
{
    if(thread)
    {
        thread->return_data = thread->entry_point((void *)thread->return_data);
        thread->state = K_PROC_THREAD_STATE_FINISHED;
        k_proc_Yield();
    }
}

void k_proc_RunThread(struct k_proc_thread_t *thread)
{
    k_apic_StartTimer(0x1fff);
    thread->state = K_PROC_THREAD_STATE_RUNNING;
    k_proc_SwitchToThread(thread);
    switch(thread->state)
    {
        case K_PROC_THREAD_STATE_FINISHED:
            if(thread->prev)
            {
                thread->prev->next = thread->next;
            }
            else
            {
                thread->process->threads = thread->next;
            }

            if(thread->next)
            {
                thread->next->prev = thread->prev;
            }
            else
            {
                thread->process->last_thread = thread->prev;
            }

            thread->prev = NULL;
            thread->next = k_proc_finished_threads;
            k_proc_finished_threads = thread;
        break;

        case K_PROC_THREAD_STATE_RUNNING:
            thread->state = K_PROC_THREAD_STATE_READY;
        break;
    }

    k_apic_StopTimer();
    k_apic_EndOfInterrupt();
}