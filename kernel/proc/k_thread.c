#include "k_thread.h"
#include "k_proc.h"
#include "../atm/k_atm.h"
#include "../cont/k_objlist.h"
#include "../mem/alloc.h"
#include "../timer/k_apic.h"


// extern k_atm_spnl_t k_proc_threads_spinlock;
// extern struct k_cont_objlist_t k_proc_threads;
// extern struct k_proc_thread_t *k_proc_current_thread;
// extern struct k_proc_thread_t k_proc_scheduler_thread;

// extern struct k_proc_thread_t *k_proc_finished_list;
// extern struct k_proc_thread_t *k_proc_delete_queue;

extern struct k_proc_thread_t *k_proc_wait_list;
// extern struct k_proc_thread_t *k_proc_local_delete_list;
extern struct k_proc_thread_t *k_proc_suspended_threads;

extern k_atm_spnl_t k_proc_ready_queue_spinlock;
extern struct k_proc_thread_t *k_proc_ready_queue;
extern struct k_proc_thread_t *k_proc_ready_queue_last;

extern struct k_proc_core_state_t k_proc_core_state;

struct k_proc_thread_t *k_proc_CreateThread(uintptr_t (*thread_fn)(void *data), void *data)
{
    // k_atm_SpinLock(&k_proc_threads_spinlock);
    // uint32_t thread_id = k_cont_AllocObjListElement(&k_proc_threads);
    // k_atm_SpinUnlock(&k_proc_threads_spinlock);
    // k_proc_EnablePreemption();

    k_atm_SpinLock(&k_proc_core_state.thread_pool.spinlock);
    uint32_t thread_id = k_cont_AllocObjListElement(&k_proc_core_state.thread_pool.threads);
    struct k_proc_thread_t *thread = k_cont_GetObjListElement(&k_proc_core_state.thread_pool.threads, thread_id);
    k_atm_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);
    
    // struct k_proc_thread_t *thread = k_cont_GetObjListElement(&k_proc_threads, thread_id);
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();

    uint32_t ring = current_process->ring;

    thread->id = thread_id;
    thread->core = 0;
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
        current_process->last_thread->process_next = thread;
    }

    thread->process_prev = current_process->last_thread;
    current_process->last_thread = thread;

    // thread->wait_queue = NULL;
    thread->wait_thread = NULL;

    /* thread stack gets allocated from the process heap */
    uintptr_t stack = (uintptr_t)k_mem_Malloc(&current_thread->heap, K_PROC_THREAD_WORK_STACK_SIZE, 4);
    thread->stack_base = stack;
    thread->entry_point = thread_fn;

    stack += K_PROC_THREAD_WORK_STACK_SIZE;

    thread->start_esp = (uintptr_t *)stack;
    thread->current_esp = thread->start_esp;
    thread->state = K_PROC_THREAD_STATE_CREATED;
    thread->page_dir = current_process->page_map;
    // thread->refs = 0;

    uintptr_t start_ebp = (uintptr_t)thread->start_esp;
    uint32_t code_seg = K_CPU_SEG_SEL(2, 0, 0);
    uint32_t stack_seg = K_CPU_SEG_SEL(1, 0, 0);
    uint32_t data_seg = K_CPU_SEG_SEL(1, 0, 0);

    if(ring)
    {
        /* threads not in ring 0 will have a dedicated "stack" to store its context,
        while threads in ring 0 will store its context at the very end of their work stack */
        uintptr_t state_block = (uintptr_t)k_mem_Malloc(&k_proc_core_state.scheduler_thread.heap, K_PROC_THREAD_KERNEL_STACK_SIZE, 4);
        state_block += K_PROC_THREAD_KERNEL_STACK_SIZE;
        thread->current_esp = (uintptr_t *)state_block;

        code_seg = K_CPU_SEG_SEL(4, 3, 0);
        stack_seg = K_CPU_SEG_SEL(3, 3, 0);
        data_seg = K_CPU_SEG_SEL(3, 3, 0);

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
        *thread->current_esp = stack_seg;
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
    *thread->current_esp = code_seg;
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
    *thread->current_esp = data_seg;
    /* es */
    thread->current_esp--;
    *thread->current_esp = data_seg;
    /* fs */
    thread->current_esp--;
    *thread->current_esp = data_seg;

    // k_proc_SetThreadReady(thread->id);

    // return K_PROC_THREAD_HANDLE(thread->core, thread->id);
    return thread;
}

void k_proc_DetachThread(struct k_proc_thread_t *thread)
{
    // uint32_t thread_id = K_PROC_THREAD_INDEX(handle);
    // struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);

    if(K_PROC_THREAD_VALID(thread) && !(thread->flags & K_PROC_THREAD_FLAG_DETACHED))
    {
        thread->flags |= K_PROC_THREAD_FLAG_DETACHED;

        if(thread->state == K_PROC_THREAD_STATE_FINISHED)
        {
            // if(k_atm_TrySpinLock(&k_proc_threads_spinlock))
            // {
            //     k_proc_DestroyThread(thread->id);
            //     k_atm_SpinUnlock(&k_proc_threads_spinlock);
            // }
            // else
            // {
            /* we'll need to alter the local delete list of this core
            using atomic operations, because chances are big this isn't
            the scheduler thread. */
            struct k_proc_thread_t *old_head;
            do
            {
                thread->queue_next = k_proc_core_state.delete_list;
            }
            while(!k_atm_CmpXcgh((uintptr_t *)&k_proc_core_state.delete_list, (uintptr_t )thread->queue_next, 
                                    (uintptr_t )thread, (uintptr_t *)&old_head));
            // }
        }
    }
}

struct k_proc_thread_t *k_proc_GetThread(uint32_t handle)
{
    struct k_proc_thread_t *thread = NULL;
    uint32_t thread_id = K_PROC_THREAD_INDEX(handle);
    k_atm_SpinLock(&k_proc_core_state.thread_pool.spinlock);
    thread = k_cont_GetObjListElement(&k_proc_core_state.thread_pool.threads, thread_id);
    k_atm_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);

    if(!K_PROC_THREAD_VALID(thread))
    {
        thread = NULL;
    }

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
        uint32_t thread_id = thread->id;
        thread->id = K_PROC_INVALID_THREAD_ID;
        struct k_proc_process_t *current_process = thread->process;

        if(thread->process_prev)
        {
            thread->process_prev->process_next = thread->process_next;
        }
        else
        {
            thread->process->threads = thread->process_next;
        }

        if(thread->process_next)
        {
            thread->process_next->process_prev = thread->process_prev;
        }
        else
        {
            thread->process->last_thread = thread->process_prev;
        }

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
        k_mem_Free(&k_proc_core_state.scheduler_thread.heap, kernel_stack);
        k_atm_SpinLock(&k_proc_core_state.thread_pool.spinlock);
        k_cont_FreeObjListElement(&k_proc_core_state.thread_pool.threads, thread_id);
        k_atm_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);
    }
}

void k_proc_KillThread(struct k_proc_thread_t *thread)
{
    // struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);

    if(K_PROC_THREAD_VALID(thread))
    {

    }
}

void k_proc_SuspendThread(struct k_proc_thread_t *thread)
{
    (void)thread;
    // (void)thread_id;
}

// void k_proc_SetThreadReady(uint32_t thread_id)
// {
//     struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);

//     if(thread)
//     {
//         k_atm_SpinLock(&k_proc_ready_queue_spinlock);
//         if(!k_proc_ready_queue)
//         {
//             k_proc_ready_queue = thread;
//         }
//         else
//         {
//             k_proc_ready_queue_last->queue_next = thread;
//         }
//         thread->queue_prev = k_proc_ready_queue_last;
//         k_proc_ready_queue_last = thread;
//         k_atm_SpinUnlock(&k_proc_ready_queue_spinlock);
//     }
// }

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

        if(wait_thread->state != K_PROC_THREAD_STATE_FINISHED)
        {
            current_thread->wait_thread = wait_thread;
            current_thread->state = K_PROC_THREAD_STATE_WAITING;    
            k_proc_Yield();
        }

        *value = wait_thread->return_data;
        k_proc_DetachThread(wait_thread);

        return K_STATUS_OK;
    }

    return K_STATUS_INVALID_THREAD;
}

void k_proc_ReadyQueuePush(struct k_proc_thread_t *thread)
{
    if(thread && thread->state == K_PROC_THREAD_STATE_READY)
    {

    }   
}

// void k_proc_DeletedQueuePush(struct k_proc_thread_t *thread)
// {
//     if(thread)
//     {

//     }
// }

// void k_proc_QueueDetachedThread(struct k_proc_thread_t *thread)
// {
//     if(thread && (thread->flags & K_PROC_THREAD_FLAG_DETACHED))
//     {
//         struct k_proc_thread_t *old_head;
//         do
//         {
//             thread->queue_next = k_proc_detached_threads;
//         }
//         while(!k_atm_CmpXcgh((uintptr_t *)&k_proc_detached_threads, (uintptr_t)thread->queue_next, (uintptr_t)thread, (uintptr_t *)&old_head));
//     }
// }

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}

void k_proc_RunThreadCallback(struct k_proc_thread_t *thread)
{
    if(thread)
    {
        thread->return_data = thread->entry_point((void *)thread->return_data);
        thread->state = K_PROC_THREAD_STATE_RETURNED;
        k_proc_Yield();
    }
}

void k_proc_RunThread(struct k_proc_thread_t *thread)
{
    thread->state = K_PROC_THREAD_STATE_RUNNING;
    k_apic_StartTimer(0x1fff);
    k_proc_SwitchToThread(thread);
    k_apic_StopTimer();
    k_apic_EndOfInterrupt();

    switch(thread->state)
    {
        case K_PROC_THREAD_STATE_RETURNED:

            thread->state = K_PROC_THREAD_STATE_FINISHED;

            if(thread->flags & K_PROC_THREAD_FLAG_DETACHED)
            {
                /* no need to for atomic operations here. It's guaranteed no other 
                thread is currently touching this list, because it's local to the 
                core executing this thread, and this thread is the scheduler thread. */
                thread->queue_next = k_proc_core_state.delete_list;
                k_proc_core_state.delete_list = thread;
            }
        break;

        case K_PROC_THREAD_STATE_WAITING:
        {
            struct k_proc_thread_t *old_head;
            do
            {
                thread->queue_next = k_proc_wait_list;
            }
            while(!k_atm_CmpXcgh((uintptr_t *)&k_proc_wait_list, (uintptr_t)thread->queue_next, 
                                 (uintptr_t)thread, (uintptr_t *)&old_head));
        }
        break;

        case K_PROC_THREAD_STATE_RUNNING:
            thread->state = K_PROC_THREAD_STATE_READY;
        break;
    }
}