#include <stddef.h>
#include "k_proc.h"
#include "k_thread.h"
#include "../k_term.h"
#include "../timer/k_apic.h"
#include "../k_int.h"
#include "../k_rng.h"
#include "../mem/mem.h"
#include "../cont/k_objlist.h"
#include "../atm/k_atm.h"
#include "../../libdg/container/dg_slist.h"
#include "../../libdg/malloc/dg_malloc.h"

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
// struct k_proc_thread_t k_proc_scheduler_thread;



// k_atm_spnl_t k_proc_ready_queue_spinlock = 0;
// struct k_proc_thread_t *k_proc_ready_queue = NULL;
// struct k_proc_thread_t *k_proc_ready_queue_last = NULL;

// struct k_proc_thread_t *k_proc_finished_list = NULL;

k_atm_spnl_t k_proc_wait_list_spinlock = 0;
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

extern void *k_proc_PreemptCurrentThread;
extern uint32_t k_mem_page_map;
 
void k_proc_Init()
{
    struct k_proc_thread_t *scheduler_thread = &k_proc_core_state.scheduler_thread;
    // // k_proc_current_process = &k_proc_kernel_process;
    k_proc_core_state.current_thread = scheduler_thread;
    scheduler_thread->process = &k_proc_scheduler_process;
    k_proc_scheduler_process.page_map = k_mem_page_map;

    for(uint32_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        scheduler_thread->heap.buckets[bucket_index].first_chunk = NULL;
        scheduler_thread->heap.buckets[bucket_index].last_chunk = NULL;
    }
    scheduler_thread->page_dir = k_proc_scheduler_process.page_map;
    k_proc_scheduler_process.pid = 0;
    k_proc_scheduler_process.ring = 0;

    // k_proc_threads = k_cont_CreateObjList(sizeof(struct k_proc_thread_t), 512, &k_proc_scheduler_thread.heap, 1);
    // // k_proc_processes = dg_StackListCreate(sizeof(struct k_proc_process_t), 512);
    // // k_proc_threads = dg_StackListCreate(sizeof(struct k_proc_thread_t), 512);

    k_proc_core_state.tss = k_mem_Malloc(&scheduler_thread->heap, sizeof(struct k_cpu_tss_t), 8);
    k_proc_core_state.tss->ss0 = K_CPU_SEG_SEL(1, 0, 0);
    k_proc_gdt[5] = K_CPU_SEG_DESC((uint32_t)k_proc_core_state.tss, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_cpu_Lgdt(k_proc_gdt, 7, K_CPU_SEG_SEL(2, 0, 0));
    k_cpu_Ltr(K_CPU_SEG_SEL(5, 3, 0));

    k_int_SetInterruptHandler(K_PROC_THREAD_PREEMPT_VECTOR, (uintptr_t)&k_proc_PreemptCurrentThread, K_CPU_SEG_SEL(2, 0, 0), 3);
    k_apic_WriteReg(K_APIC_REG_LVT_TIMER, (k_apic_ReadReg(K_APIC_REG_LVT_TIMER) | (K_PROC_THREAD_PREEMPT_VECTOR & 0xff) ) & (0xfff8ffff));
    k_apic_WriteReg(K_APIC_REG_DIV_CONFIG, k_apic_ReadReg(K_APIC_REG_DIV_CONFIG) & (~0xb));
    k_apic_WriteReg(K_APIC_REG_SPUR_INT_VEC, k_apic_ReadReg(K_APIC_REG_SPUR_INT_VEC) | 34);
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
    uint32_t thread_index = 0;
    k_proc_core_state.current_thread = &k_proc_core_state.scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_Yield();

    while(1)
    {
        // if(k_atm_TrySpinLock(&k_proc_core_state.thread_pool.spinlock))
        // {
        //     struct k_proc_thread_t *deleted_thread = k_proc_core_state.delete_list;
        //     struct k_proc_thread_t *old_head;
        //     k_atm_Xcgh32(&k_proc_core_state.delete_list, NULL, &old_head);

        //     while(deleted_thread)
        //     {
        //         struct k_proc_thread_t *next_thread = deleted_thread->queue_next;
        //         k_proc_DestroyThread(deleted_thread);
        //         deleted_thread = next_thread;
        //     }

        //     k_atm_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);
        // }

        // struct k_proc_thread_t *waiting_thread = k_proc_wait_list;
        // struct k_proc_thread_t *old_head;
        // k_atm_Xcgh32(&k_proc_wait_list, NULL, &old_head);
        // struct k_proc_thread_t *prev_thread = NULL;
        // struct k_proc_thread_t *resume_threads = NULL;

        // while(waiting_thread)
        // {
        //     struct k_proc_thread_t *next_thread = waiting_thread->queue_next;
        //     if(waiting_thread->wait_thread->state == K_PROC_THREAD_STATE_FINISHED)
        //     {
        //         if(prev_thread)
        //         {
        //             prev_thread->queue_next = next_thread;
        //         }

        //         waiting_thread->queue_next = resume_threads;
        //         resume_threads = waiting_thread;
        //     }
        //     else
        //     {
        //         prev_thread = waiting_thread;
        //     }

        //     waiting_thread = next_thread;
        // }

        // if(k_proc_threads.cursor)
        // {
        //     uint32_t thread_id = (thread_index + k_rng_Rand()) % k_proc_threads.cursor;
        //     struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);
        //     if(thread && thread->state == K_PROC_THREAD_STATE_READY)
        //     {
        //         k_proc_RunThread(thread);
        //     }
        // }
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