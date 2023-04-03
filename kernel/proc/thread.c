#include "thread.h"
#include "proc.h"
#include "../rt/atm.h"
// #include "../cont/k_objlist.h"
#include "../int/apic.h"
#include "../rt/mem.h"
#include "../rt/alloc.h"
#include "../rt/queue.h"
#include "../sys/proc/thread.h"
#include "../mem/mngr.h"
#include "../mem/pmap.h"

// extern struct k_proc_thread_t *k_proc_thread_wait_list;
// extern struct k_proc_thread_t *k_proc_io_wait_list;
// extern struct k_proc_thread_t *k_proc_suspended_threads;
extern struct k_proc_thread_t *k_proc_cond_wait_list;

// extern k_rt_spnl_t k_proc_ready_queue_spinlock;
// extern struct k_proc_thread_t *k_proc_ready_queue;
// extern struct k_proc_thread_t *k_proc_ready_queue_last;
extern struct k_rt_queue_t k_proc_ready_queue;

extern struct k_proc_core_state_t k_proc_core_state;
extern uint32_t k_proc_page_map;
extern void *k_proc_SetupUserStack_a;

struct k_proc_thread_t *k_proc_CreateKernelThread(k_proc_thread_func_t entry_point, void *user_data)
{
    struct k_proc_thread_init_t thread_init = {};
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    thread_init.process = current_thread->process;
    thread_init.entry_point = entry_point;
    thread_init.user_data = user_data;
    thread_init.ring0_stack_base = k_mem_AllocVirtualRange(K_PROC_THREAD_KERNEL_STACK_SIZE);
    thread_init.ring0_stack_page = k_mem_AllocPhysicalPage(0);
    k_mem_MapLinearAddress(thread_init.ring0_stack_base, thread_init.ring0_stack_page, K_MEM_PENTRY_FLAG_READ_WRITE);
    return k_proc_CreateThread(&thread_init);
}

struct k_proc_thread_t *k_proc_CreateThread(struct k_proc_thread_init_t *init)
{
    struct k_proc_thread_pool_t *thread_pool = &k_proc_core_state.thread_pool;
    struct k_proc_thread_t *thread = NULL;
    struct k_proc_thread_t *old = NULL;

    if(!init->ring0_stack_page || !init->ring0_stack_base || !init->entry_point)
    {
        return NULL;
    }

    do
    {
        thread = thread_pool->threads;

        if(!thread)
        {
            union k_proc_thread_page_t *thread_page = k_rt_Malloc(4096, 4096);
            thread_page->count = K_PROC_THREAD_PAGE_THREAD_COUNT;
            for(uint32_t thread_index = 0; thread_index < K_PROC_THREAD_PAGE_THREAD_COUNT - 1; thread_index++)
            {
                struct k_proc_thread_t *thread = &thread_page->threads[thread_index];
                thread->queue_next = &thread_page->threads[thread_index + 1];
            }

            struct k_proc_thread_t *last_thread = &thread_page->threads[K_PROC_THREAD_PAGE_THREAD_COUNT - 1];
            last_thread->queue_next = NULL;

            k_rt_SpinLock(&thread_pool->spinlock);
            thread_page->next = thread_pool->pages;
            thread_pool->pages = thread_page;
            thread_pool->threads = &thread_page->threads[0];
            k_rt_SpinUnlock(&thread_pool->spinlock);

            thread = thread_pool->threads;

        }
    }
    while(!k_rt_CmpXchg32(&thread_pool->threads, thread, thread->queue_next, &old));

    // struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    // struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    struct k_proc_process_t *process = init->process;

    thread->process = process;

    for(uint32_t bucket_index = 0; bucket_index < K_RT_SMALL_BUCKET_COUNT; bucket_index++)
    {
        thread->heap.buckets[bucket_index].first_chunk = NULL;
        thread->heap.buckets[bucket_index].last_chunk = NULL;
    }

    thread->heap.free_pages = NULL;
    thread->heap.big_heap = &process->heap;
    thread->heap.spinlock = 0;
    thread->wait_thread = NULL;
    thread->queue_next = NULL;
    thread->state = K_PROC_THREAD_STATE_CREATED;
    thread->flags = 0;
    thread->condition = 0;
    thread->wait_condition = NULL;

    // if(!current_process->threads)
    // {
    //     current_process->threads = thread;
    // }
    // else
    // {
    //     current_process->last_thread->process_next = thread;
    // }
    // current_process->last_thread = thread;

    uintptr_t kernel_stack_base;

    if(init->user_stack_base)
    {
        /* user thread */
        kernel_stack_base = (uintptr_t)k_mem_AllocVirtualRange(K_PROC_THREAD_KERNEL_STACK_SIZE);
        k_mem_MapLinearAddress(kernel_stack_base, init->ring0_stack_page, K_MEM_PENTRY_FLAG_READ_WRITE);
    }
    else
    {
        /* kernel thread */
        kernel_stack_base = init->ring0_stack_base;
    }


    thread->kernel_stack_offset = kernel_stack_base - init->ring0_stack_base;
    uint32_t current_sp = K_PROC_THREAD_KERNEL_STACK_SIZE / sizeof(uintptr_t);
    uintptr_t *ring0_stack = (uintptr_t *)init->ring0_stack_base;
    uintptr_t *kernel_stack = (uintptr_t *)kernel_stack_base;

    // uintptr_t *kernel_stack = (kernel_stack_base + K_PROC_THREAD_KERNEL_STACK_SIZE;
    // thread->current_sp = (uintptr_t *)kernel_stack;
    thread->start_sp = (uintptr_t)(ring0_stack + current_sp);
    // k_sys_TerminalPrintf("%x\n", thread->start_sp);
    // thread->current_pmap = process->page_map;

    uint32_t code_seg = K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0);
    uint32_t data_seg = K_CPU_SEG_SEL(K_PROC_R0_DATA_SEG, 0, 0);
    uintptr_t start_eip;

    // if(process->ring)
    // {
    //     thread->current_sp--;
    //     *thread->current_sp = init->user_data;
    //     thread->current_sp--;
    //     *thread->current_sp = (uintptr_t)init->entry_point;
    //     thread->current_sp--;
    //     *thread->current_sp = init->user_stack;
    //     thread->current_sp--;
    //     *thread->current_sp = 0;
    //     start_eip = (uintptr_t)&k_proc_SetupUserStack_a;
    // }
    // else
    {
        /* r0 thread */
        current_sp--;
        kernel_stack[current_sp] = init->user_data;
        current_sp--;
        kernel_stack[current_sp] = (uintptr_t)init->entry_point;
        current_sp--;
        kernel_stack[current_sp] = 0;
        start_eip = (uintptr_t)k_proc_StartKernelThread;
    }

    /* eflags */
    current_sp--;
    kernel_stack[current_sp] = K_CPU_STATUS_REG_INIT_VALUE | K_CPU_STATUS_FLAG_INT_ENABLE;
    /* cs */
    current_sp--;
    kernel_stack[current_sp] = code_seg;
    /* eip */
    current_sp--;
    kernel_stack[current_sp] = start_eip;
    /* eax */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* ebx */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* ecx */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* edx */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* esi */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* edi */
    current_sp--;
    kernel_stack[current_sp] = 0;
    /* ebp */
    current_sp--;
    kernel_stack[current_sp] = (uintptr_t)(thread->start_sp - 1);
    /* ds */
    current_sp--;
    kernel_stack[current_sp] = data_seg;
    /* es */
    current_sp--;
    kernel_stack[current_sp] = data_seg;
    /* fs */
    current_sp--;
    kernel_stack[current_sp] = data_seg;
    /* page map */
    current_sp--;
    kernel_stack[current_sp] = process->page_map;

    thread->current_pmap = process->page_map;
    thread->current_sp = (uintptr_t)(kernel_stack + current_sp);
    // thread->current_sp--;
    // *thread->current_sp = data_seg;

    if(!process->main_thread)
    {
        process->main_thread = thread;
    }

    k_proc_QueueReadyThread(thread);

    return thread;
}

void k_proc_DetachThread(struct k_proc_thread_t *thread)
{
    if(thread && !(thread->flags & K_PROC_THREAD_FLAG_DETACHED))
    {
        thread->flags |= K_PROC_THREAD_FLAG_DETACHED;

        if(thread->state == K_PROC_THREAD_STATE_TERMINATED)
        {
            thread->state = K_PROC_THREAD_STATE_DETACHED;
            k_proc_QueueDetachedThread(thread);
        }
    }
}

struct k_proc_thread_t *k_proc_GetThread(uint32_t handle)
{
    struct k_proc_thread_t *thread = NULL;
    // uint32_t thread_id = K_PROC_THREAD_INDEX(handle);
    // k_atm_SpinLock(&k_proc_core_state.thread_pool.spinlock);
    // thread = k_cont_GetObjListElement(&k_proc_core_state.thread_pool.threads, thread_id);
    // k_atm_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);

    // if(!K_PROC_THREAD_VALID(thread))
    // {
    //     thread = NULL;
    // }

    return thread;
}

struct k_proc_thread_t *k_proc_GetCurrentThread()
{
    return k_proc_core_state.current_thread;
}

void k_proc_DestroyThread(struct k_proc_thread_t *thread)
{
    if(K_PROC_THREAD_VALID(thread))
    {
        thread->state = K_PROC_THREAD_STATE_INVALID;

        // if(thread->process_prev)
        // {
        //     thread->process_prev->process_next = thread->process_next;
        // }
        // else
        // {
        //     thread->process->threads = thread->process_next;
        // }

        // if(thread->process_next)
        // {
        //     thread->process_next->process_prev = thread->process_prev;
        // }
        // else
        // {
        //     thread->process->last_thread = thread->process_prev;
        // }

        uintptr_t kernel_stack_base = ((uintptr_t)thread->start_sp + thread->kernel_stack_offset) - K_PROC_THREAD_KERNEL_STACK_SIZE;
        uintptr_t kernel_stack_page = k_mem_LinearAddressPhysicalPage(kernel_stack_base);

        k_mem_UnmapLinearAddress(kernel_stack_base);
        k_mem_FreeVirtualRange((void *)kernel_stack_base);
        k_mem_FreePhysicalPages(kernel_stack_page);

        // union k_proc_thread_page_t *thread_page = (union k_proc_thread_page_t *)((uintptr_t)thread & K_MEM_4KB_ADDRESS_MASK);
        struct k_proc_thread_pool_t *thread_pool = &k_proc_core_state.thread_pool;
        struct k_proc_thread_t *old_head;
        do
        {
            thread->queue_next = thread_pool->threads;
        }
        while(!k_rt_CmpXchg32((uint32_t *)&thread_pool->threads, (uint32_t)thread->queue_next, (uint32_t)thread, (uint32_t *)&old_head));
        // k_printf("free %x               \n", thread);
    }

}

void k_proc_SuspendThread(struct k_proc_thread_t *thread)
{
    (void)thread;
    // (void)thread_id;
}

uint32_t k_proc_WaitThread(struct k_proc_thread_t *thread, uintptr_t *value)
{
    struct k_proc_thread_t *wait_thread = thread;
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    if(K_PROC_THREAD_VALID(wait_thread) && current_thread != wait_thread)
    {
        if(wait_thread->flags & K_PROC_THREAD_FLAG_DETACHED)
        {
            return K_STATUS_DETACHED_THREAD;
        }

        if(wait_thread->state != K_PROC_THREAD_STATE_TERMINATED)
        {
            // current_thread->wait_thread = wait_thread;
            // current_thread->state = K_PROC_THREAD_STATE_WAITING;
            // k_proc_YieldThread();
            k_proc_WaitCondition(&wait_thread->condition);
        }

        *value = wait_thread->return_data;
        current_thread->wait_condition = NULL;
        k_proc_DetachThread(wait_thread);

        return K_STATUS_OK;
    }
    // k_printf("%x is invalid       \n", thread);
    return K_STATUS_INVALID_THREAD;
}

// uint32_t k_proc_WaitStream(struct k_io_stream_t *stream)
// {
//     struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
// 
//     if(stream)
//     {
//         // current_thread->wait_stream = stream;
//         // current_thread->state = K_PROC_THREAD_STATE_IO_BLOCKED;
//         // k_sys_TerminalPrintf("Thread %x will wait on stream %x\n", current_thread, stream);
//         // k_proc_YieldThread();
//         // k_sys_TerminalPrintf("Thread %x resumed after waiting on stream %x\n", current_thread, stream);
//         k_proc_WaitCondition(&stream->condition, K_PROC_THREAD_STATE_IO_BLOCKED);
// 
//         return K_STATUS_OK;
//     }
// 
//     return K_STATUS_INVALID_STREAM;
// }

uint32_t k_proc_WaitCondition(k_rt_cond_t *condition)
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    
    if(condition && !(*condition))
    {
        current_thread->wait_condition = condition;
        current_thread->state = K_PROC_THREAD_STATE_COND_WAIT;
        k_proc_YieldThread();
    }
    
    return 0;
}

void k_proc_YieldThread()
{
    k_proc_SwitchToThread(NULL);
}

void k_proc_StartThread(k_proc_thread_func_t entry_point, void *user_data)
{
    uintptr_t return_value = entry_point(user_data);
    k_proc_TerminateThread(return_value);
}

void k_proc_TerminateThread(uintptr_t return_value)
{
    struct k_proc_thread_t *thread = k_proc_GetCurrentThread();
    // k_printf("terminate %x              \n", thread);

    if(thread->flags & K_PROC_THREAD_FLAG_DETACHED)
    {
        thread->state = K_PROC_THREAD_STATE_DETACHED;
    }
    else
    {
        thread->state = K_PROC_THREAD_STATE_TERMINATED;
        thread->return_data = return_value;
    }
    
    k_rt_SignalCondition(&thread->condition);
    k_proc_YieldThread();
}

void k_proc_RunThread(struct k_proc_thread_t *thread)
{
    thread->state = K_PROC_THREAD_STATE_RUNNING;
    k_apic_StartTimer(0x1fff);
    if(thread->current_pmap != k_proc_page_map)
    {
        __asm__ volatile ("nop\n nop\n");
    }

    // k_sys_TerminalPrintf("run thread %x\n", thread);
    k_proc_SwitchToThread(thread);
    k_apic_StopTimer();
    k_apic_EndOfInterrupt();

    if(thread != k_proc_core_state.cleanup_thread)
    {
        switch(thread->state)
        {
            case K_PROC_THREAD_STATE_DETACHED:
                k_proc_QueueDetachedThread(thread);
            break;

            // case K_PROC_THREAD_STATE_WAITING:
            //     k_proc_QueueWaitingThread(thread);
            // break;
            // 
            // case K_PROC_THREAD_STATE_IO_BLOCKED:
            //     k_proc_QueueIOBlockedThread(thread);
            // break;
            
            case K_PROC_THREAD_STATE_COND_WAIT:
                k_proc_QueueWaitingThread(thread);
            break;

            case K_PROC_THREAD_STATE_RUNNING:
                k_proc_QueueReadyThread(thread);
            break;
        }
    }
    else
    {
        thread->state = K_PROC_THREAD_STATE_READY;
    }

}

void k_proc_SwitchToThread(struct k_proc_thread_t *thread)
{
    k_proc_core_state.next_thread = thread;
    __asm__ volatile ("int 38");
}

void k_proc_QueueReadyThread(struct k_proc_thread_t *thread)
{
    if(K_PROC_THREAD_VALID(thread))
    {
        // struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
        // struct k_proc_thread_t *ready_thread = thread;
        // struct k_proc_thread_t *last_thread = NULL;
        // k_sys_TerminalPrintf("queue thread %x\n", thread);
        while(thread)
        {
            struct k_proc_thread_t *next_thread = thread->queue_next;
            thread->state = K_PROC_THREAD_STATE_READY;
            k_rt_QueuePush(&k_proc_ready_queue, thread);
            thread = next_thread;
            // last_thread = ready_thread;
            // ready_thread = ready_thread->queue_next;
        }
        // k_rt_SpinLock(&k_proc_ready_queue_spinlock);
        // // while(!k_atm_TrySpinLock(&k_proc_ready_queue_spinlock))
        // // {
        // //     k_printf("%x tried lock       \n", current_thread);
        // // }
        // if(!k_proc_ready_queue)
        // {
        //     k_proc_ready_queue = thread;
        // }
        // else
        // {
        //     k_proc_ready_queue_last->queue_next = thread;
        // }
        // k_proc_ready_queue_last = last_thread;

        // k_rt_SpinUnlock(&k_proc_ready_queue_spinlock);
    }
}

void k_proc_QueueWaitingThread(struct k_proc_thread_t *thread)
{
    if(K_PROC_THREAD_VALID(thread) && thread != k_proc_core_state.cleanup_thread)
    {
        // thread->state = K_PROC_THREAD_STATE_WAITING;
        struct k_proc_thread_t *old_head;
        do
        {
            thread->queue_next = k_proc_cond_wait_list;
        }
        while(!k_rt_CmpXchg((uintptr_t *)&k_proc_cond_wait_list, (uintptr_t)thread->queue_next, (uintptr_t)thread, (uintptr_t *)&old_head));
    }
}

// void k_proc_QueueIOBlockedThread(struct k_proc_thread_t *thread)
// {
//     if(K_PROC_THREAD_VALID(thread) && thread != k_proc_core_state.cleanup_thread)
//     {
//         thread->state = K_PROC_THREAD_STATE_IO_BLOCKED;
//         struct k_proc_thread_t *old_head;
//         do
//         {
//             thread->queue_next = k_proc_io_wait_list;
//         }
//         while(!k_rt_CmpXchg((uintptr_t *)&k_proc_io_wait_list, (uintptr_t)thread->queue_next, (uintptr_t)thread, (uintptr_t *)&old_head));
//     }
// }

void k_proc_QueueDetachedThread(struct k_proc_thread_t *thread)
{
    if(K_PROC_THREAD_VALID(thread) && thread->state == K_PROC_THREAD_STATE_DETACHED && thread != k_proc_core_state.cleanup_thread)
    {
        struct k_proc_thread_t **delete_list = &k_proc_core_state.delete_list;
        struct k_proc_thread_t *old_head;
        do
        {
            thread->queue_next = k_proc_core_state.delete_list;
        }
        while(!k_rt_CmpXchg((uintptr_t *)delete_list, (uintptr_t )thread->queue_next, (uintptr_t )thread, (uintptr_t *)&old_head));
    }
}
