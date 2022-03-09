#include <stddef.h>
#include "proc.h"
#include "thread.h"
#include "../sys/term.h"
#include "../int/apic.h"
#include "../int/int.h"
#include "../k_rng.h"
#include "../mem/mem.h"
#include "../mem/pmap.h"
#include "../rt/alloc.h"
#include "../rt/queue.h"
// #include "../cont/k_objlist.h"
#include "../rt/atm.h"
// #include "../../libdg/container/dg_slist.h"
// #include "../../libdg/malloc/dg_malloc.h"

// struct dg_slist_t k_proc_processes;
// struct dg_slist_t k_proc_threads;

// union k_proc_thread_list_t *k_proc_thread_lists;
// uint32_t k_proc_thread_id = 0;

// k_atm_spnl_t k_proc_threads_spinlock;
// struct k_cont_objlist_t k_proc_threads;

// struct k_proc_thread_t *k_proc_finished_threads = NULL;

struct k_proc_process_t *k_proc_processes = NULL;
// struct k_proc_process_t *k_proc_last_process = NULL;
// struct k_proc_process_t *k_proc_current_process = NULL;



// struct k_proc_thread_t *k_proc_current_thread = NULL;
// struct k_proc_thread_t *k_proc_next_thread = NULL;

struct k_proc_process_t k_proc_scheduler_process;
struct k_proc_process_t *k_proc_active_process;
// struct k_proc_thread_queue_t *k_proc_ready_queue;
struct k_proc_thread_queue_t *k_proc_wait_queue;
// struct k_proc_thread_t k_proc_scheduler_thread;


struct k_rt_queue_t k_proc_ready_queue;

// k_rt_spnl_t k_proc_ready_queue_spinlock = 0;
// struct k_proc_thread_t *k_proc_ready_queue = NULL;
// struct k_proc_thread_t *k_proc_ready_queue_last = NULL;

// struct k_proc_thread_t *k_proc_finished_list = NULL;

k_rt_spnl_t k_proc_wait_list_spinlock = 0;
struct k_proc_thread_t *k_proc_wait_list = NULL;


/* each core will have on of those */
// struct k_proc_thread_t *k_proc_local_delete_list = NULL;
// struct k_proc_thread_t *k_proc_suspended_threads = NULL;

// struct k_proc_thread_queue_t k_proc_thread_queues[];


// struct k_proc_thread_t *k_proc_suspended_queue = NULL;

// k_atm_spnl_t k_proc_spinlock = 0;

// #define K_PROC_THREAD_STACK_PAGE_COUNT 4

// struct k_cpu_tss_t *k_proc_tss = NULL;

struct k_proc_core_state_t k_proc_core_state;
struct k_proc_thread_t *k_proc_cleanup_thread;
struct k_cpu_seg_desc_t k_proc_gdt[] = 
{ 
    K_CPU_SEG_DESC(0x00000000u, 0x000000u, 0, 0, 0, 0, 0),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),         
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EOC, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
};

// struct k_cpu_seg_desc_t k_proc_syscall_ldt;

extern void *k_proc_PreemptThread;
extern void *k_proc_StartUserThread_a;
// extern void *k_sys_SysCall;
uint32_t k_proc_page_map;
 
void k_proc_Init()
{
    struct k_proc_thread_t *scheduler_thread = &k_proc_core_state.scheduler_thread;
    // // k_proc_current_process = &k_proc_kernel_process;
    k_proc_core_state.current_thread = scheduler_thread;
    scheduler_thread->process = &k_proc_scheduler_process;
    k_proc_scheduler_process.page_map = k_proc_page_map;

    for(uint32_t bucket_index = 0; bucket_index < K_RT_SMALL_BUCKET_COUNT; bucket_index++)
    {
        scheduler_thread->heap.buckets[bucket_index].first_chunk = NULL;
        scheduler_thread->heap.buckets[bucket_index].last_chunk = NULL;
    }
    scheduler_thread->heap.big_heap = &k_proc_scheduler_process.heap;
    scheduler_thread->current_pmap = k_proc_scheduler_process.page_map;
    k_proc_scheduler_process.pid = 0;
    k_proc_scheduler_process.ring = 0;
    k_proc_scheduler_process.heap.spinlock = 0;

    k_proc_core_state.thread_pool.pages = NULL;
    k_proc_core_state.thread_pool.threads = NULL;
    // k_proc_core_state.thread_pool.threads = k_cont_CreateObjList(sizeof(struct k_proc_thread_t), 512, &scheduler_thread->heap, 1);
    // // k_proc_processes = dg_StackListCreate(sizeof(struct k_proc_process_t), 512);
    // // k_proc_threads = dg_StackListCreate(sizeof(struct k_proc_thread_t), 512);

    k_proc_core_state.tss = k_rt_Malloc(sizeof(struct k_cpu_tss_t), 8);
    k_proc_core_state.tss->ss0 = K_CPU_SEG_SEL(K_PROC_R0_DATA_SEG, 0, 0);
    k_proc_gdt[5] = K_CPU_SEG_DESC((uint32_t)k_proc_core_state.tss, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_cpu_Lgdt(k_proc_gdt, 7, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0));
    k_cpu_Ltr(K_CPU_SEG_SEL(5, 3, 0));

    k_int_SetInterruptHandler(K_PROC_PREEMPT_THREAD_VECTOR, (uintptr_t)&k_proc_PreemptThread, K_CPU_SEG_SEL(2, 0, 0), 3);
    k_int_SetInterruptHandler(K_PROC_START_USER_THREAD_VECTOR, (uintptr_t)&k_proc_StartUserThread_a, K_CPU_SEG_SEL(2, 0, 0), 0);
    // k_int_SetInterruptHandler(K_PROC_SYSCALL_VECTOR, (uintptr_t)&k_sys_SysCall, K_CPU_SEG_SEL(2, 0, 0), 3);
    k_apic_WriteReg(K_APIC_REG_LVT_TIMER, (k_apic_ReadReg(K_APIC_REG_LVT_TIMER) | (K_PROC_PREEMPT_THREAD_VECTOR & 0xff) ) & (0xfff8ffff));
    k_apic_WriteReg(K_APIC_REG_DIV_CONFIG, k_apic_ReadReg(K_APIC_REG_DIV_CONFIG) & (~0xb));
    k_apic_WriteReg(K_APIC_REG_SPUR_INT_VEC, k_apic_ReadReg(K_APIC_REG_SPUR_INT_VEC) | 34);

    k_proc_ready_queue = k_rt_QueueCreate();

    struct k_proc_thread_t *cleanup_thread = k_proc_CreateKernelThread(k_proc_CleanupThread, NULL);
    // struct k_proc_thread_t *thread = k_proc_ready_queue;
    // struct k_proc_thread_t *prev_thread = NULL;
    k_rt_QueuePop(&k_proc_ready_queue);

    // while(thread)
    // {
    //     if(thread == cleanup_thread)
    //     {
    //         break;
    //     }

    //     prev_thread = thread;
    //     thread = thread->queue_next;
    // }

    // if(!prev_thread)
    // {
    //     k_proc_ready_queue = k_proc_ready_queue->queue_next;
    // }
    // else
    // {
    //     prev_thread->queue_next = cleanup_thread->queue_next;
    // }

    // if(cleanup_thread == k_proc_ready_queue_last)
    // {
    //     k_proc_ready_queue_last = prev_thread;
    // }

    cleanup_thread->queue_next = NULL;

    k_proc_core_state.cleanup_thread = cleanup_thread;
}

uint32_t k_proc_CreateProcess(uint32_t start_address, void *image, uint32_t size)
{
    (void)start_address;
    (void)image;
    (void)size;
    return 0;
}

struct k_proc_process_t *k_proc_GetProcess(uint32_t process_id)
{
    (void)process_id;
    struct k_proc_process_t *process = NULL;

    // process = dg_StackListGetElement(&k_proc_processes, process_id);

    // if(process && process->pid == DG_INVALID_INDEX)
    // {
    //     process = NULL;
    // }

    return process;
}

struct k_proc_process_t *k_proc_GetCurrentProcess()
{
    return k_proc_core_state.current_thread->process;
}

void k_proc_RunScheduler()
{
    // uint32_t thread_index = 0;
    k_proc_core_state.current_thread = &k_proc_core_state.scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_YieldThread();

    // struct k_proc_thread_t *queued_resume_threads = NULL;

    while(1)
    {
        struct k_proc_thread_t *waiting_thread = NULL;
        struct k_proc_thread_t *resume_thread = NULL;
        struct k_proc_thread_t *last_resume_thread = NULL;
        k_rt_Xcgh32((uint32_t *)&k_proc_wait_list, (uint32_t)NULL, (uint32_t *)&waiting_thread);
        while(waiting_thread)
        {
            struct k_proc_thread_t *next_thread = waiting_thread->queue_next;

            if(waiting_thread->wait_thread->state == K_PROC_THREAD_STATE_TERMINATED)
            {
                waiting_thread->queue_next = resume_thread;
                resume_thread = waiting_thread;
            }
            else
            {
                k_proc_QueueWaitingThread(waiting_thread);
            }

            waiting_thread = next_thread;
        }

        // struct k_proc_thread_t *resume_thread = resume_threads;

        if(resume_thread)
        {
            struct k_proc_thread_t *next_resume_thread = resume_thread->queue_next;
            k_proc_QueueReadyThread(resume_thread);
            resume_thread = next_resume_thread;
            // while(resume_thread)
            // {
            //     resume_thread->state = K_PROC_THREAD_STATE_READY;
            //     last_resume_thread = resume_thread;
            //     resume_thread = resume_thread->queue_next;
            // }

            // last_resume_thread->queue_next = queued_resume_threads;

            // if(k_rt_TrySpinLock(&k_proc_ready_queue_spinlock))
            // {
            //     if(!k_proc_ready_queue)
            //     {
            //         k_proc_ready_queue = resume_threads;
            //     }
            //     else
            //     {
            //         k_proc_ready_queue_last->queue_next = resume_threads;
            //     }
            //     k_proc_ready_queue_last = last_resume_thread;

            //     k_rt_SpinUnlock(&k_proc_ready_queue_spinlock);
            //     queued_resume_threads = NULL;
            // }
            // else
            // {
            //     queued_resume_threads = resume_threads;
            // }
        }

        k_proc_RunThread(k_proc_core_state.cleanup_thread);

        struct k_proc_thread_t *next_thread = k_rt_QueuePop(&k_proc_ready_queue);

        // k_rt_SpinLock(&k_proc_ready_queue_spinlock);
        // struct k_proc_thread_t *next_thread = k_proc_ready_queue;
        // if(next_thread)
        // {
        //     k_proc_ready_queue = next_thread->queue_next;
        //     if(!k_proc_ready_queue)
        //     {
        //         k_proc_ready_queue_last = NULL;
        //     }
        //     next_thread->queue_next = NULL;
        // }
        // k_rt_SpinUnlock(&k_proc_ready_queue_spinlock);

        if(next_thread)
        {
            k_proc_RunThread(next_thread);
        }
    }
}

uintptr_t k_proc_CleanupThread(void *data)
{
    (void )data;
    
    while(1)
    {
        if(k_rt_TrySpinLock(&k_proc_core_state.thread_pool.spinlock))
        {
            uint32_t deleted_count = 0;
            struct k_proc_thread_t *deleted_thread;
            k_rt_Xcgh32((uint32_t *)&k_proc_core_state.delete_list, (uint32_t)NULL, (uint32_t *)&deleted_thread);
            while(deleted_thread)
            {
                struct k_proc_thread_t *next_thread = deleted_thread->queue_next;
                k_proc_DestroyThread(deleted_thread);
                deleted_thread = next_thread;
            }
            k_rt_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);
        }
        k_proc_YieldThread();
    }

    return 0;
}

uintptr_t k_proc_ReadyThread(void *data)
{
    (void)data;

    while(1)
    {

    }
}

void k_proc_EnablePreemption()
{
    // if(k_proc_current_thread != &k_proc_scheduler_thread)
    // {
    //     k_cpu_EnableInterrupts();
    
    //     if(k_apic_ReadReg(K_PROC_THREAD_PREEMPT_ISR_REG) & K_PROC_THREAD_PREEMPT_ISR_BIT)
    //     {
    //         k_proc_Yield();
    //     }
    // }
}

void k_proc_DisablePreemption()
{
    // k_cpu_DisableInterrupts();
}