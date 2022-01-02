#include <stddef.h>
#include "k_proc.h"
#include "../k_term.h"
#include "../k_apic.h"
#include "../k_int.h"
#include "../k_rng.h"
#include "../mem/k_mem.h"
#include "../mem/k_objlist.h"
#include "../../libdg/atomic/dg_atomic.h"
#include "../../libdg/container/dg_slist.h"
#include "../../libdg/malloc/dg_malloc.h"

// struct dg_slist_t k_proc_processes;
// struct dg_slist_t k_proc_threads;

// union k_proc_thread_list_t *k_proc_thread_lists;
// uint32_t k_proc_thread_id = 0;

struct k_mem_objlist_t k_proc_threads;

struct k_proc_process_t *k_proc_processes = NULL;
struct k_proc_process_t *k_proc_last_process = NULL;
struct k_proc_process_t *k_proc_current_process = NULL;

// struct k_proc_thread_t *k_proc_threads = NULL;
// struct k_proc_thread_t *k_proc_last_thread = NULL;
struct k_proc_thread_t *k_proc_current_thread = NULL;

struct k_proc_process_t k_proc_kernel_process;
struct k_proc_thread_t k_proc_scheduler_thread;

struct k_proc_thread_t *k_proc_next_thread = NULL;

struct k_proc_thread_t *k_proc_ready_queue = NULL;
struct k_proc_thread_t *k_proc_ready_queue_last = NULL;

struct k_proc_thread_t *k_proc_suspended_queue = NULL;

dg_spnl_t k_proc_spinlock = 0;

#define K_PROC_THREAD_STACK_PAGE_COUNT 4

struct k_cpu_tss_t *k_proc_tss;
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
 
void k_proc_Init()
{
    k_proc_current_process = &k_proc_kernel_process;
    k_proc_current_thread = &k_proc_scheduler_thread;
    k_proc_scheduler_thread.process = &k_proc_kernel_process;

    for(uint32_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        k_proc_scheduler_thread.heap.buckets[bucket_index].first_chunk = NULL;
        k_proc_scheduler_thread.heap.buckets[bucket_index].last_chunk = NULL;
    }
    k_proc_scheduler_thread.page_dir = k_proc_kernel_process.page_map.pdir_page;
    k_proc_kernel_process.pid = 0;

    k_proc_threads = k_mem_CreateObjList(sizeof(struct k_proc_thread_t), 128, &k_proc_scheduler_thread.heap, 1);
    // k_proc_processes = dg_StackListCreate(sizeof(struct k_proc_process_t), 512);
    // k_proc_threads = dg_StackListCreate(sizeof(struct k_proc_thread_t), 512);

    k_proc_tss = k_mem_Malloc(&k_proc_scheduler_thread.heap, sizeof(struct k_cpu_tss_t), 8);
    k_proc_tss->ss0 = K_CPU_SEG_SEL(1, 0, 0);
    k_proc_gdt[5] = K_CPU_SEG_DESC((uint32_t)k_proc_tss, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
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
    return k_proc_current_process;
}

// struct k_proc_thread_t *k_proc_AllocThread()
// {
//     // union k_proc_thread_list_t *thread_list;

//     // if(!k_proc_thread_lists)
//     // {
//     //     k_proc_thread_lists = k_mem_BigAlloc(&k_proc_kernel_process.heap, sizeof(union k_proc_thread_list_t), 4096);
//     //     uint32_t thread_count = sizeof(k_proc_thread_lists->threads) / sizeof(struct k_proc_thread_t);
//     //     k_proc_thread_lists->next_list = NULL;
//     //     k_proc_thread_lists->next_thread = k_proc_thread_lists->threads;
//     //     for(uint32_t thread_index = 1; thread_index < thread_count; thread_index++)
//     //     {
//     //         struct k_proc_thread_t *prev_thread = &k_proc_thread_lists->threads[thread_index - 1];
//     //         struct k_proc_thread_t *next_thread = prev_thread + 1;
//     //         // k_printf("prev thread: %x, next thread: %x\n", prev_thread, next_thread);
//     //         prev_thread->next = next_thread;
//     //         next_thread->prev = prev_thread;
//     //     }
//     // }

//     // thread_list = k_proc_thread_lists;
//     // struct k_proc_thread_t *thread = thread_list->next_thread;
//     // // k_printf("alloc thread %x\n", thread);
//     // thread_list->next_thread = thread->next;

//     // if(!thread_list->next_thread)
//     // {
//     //     k_proc_thread_lists = thread_list->next_list;
//     // }

//     // thread->next = NULL;
//     // thread->prev = NULL;

//     // return thread;
// }

// void k_proc_FreeThread(struct k_proc_thread_t *thread)
// {
//     // if(thread)
//     // {
//     //     thread->next = NULL;
//     //     thread->process = NULL;
//     //     uintptr_t thread_list_address = ((uintptr_t)thread) & 0xfffff000;
//     //     union k_proc_thread_list_t *thread_list = (union k_proc_thread_list_t *)thread_list_address;

//     //     if(!thread_list->next_thread)
//     //     {
//     //         thread_list->next_list = k_proc_thread_lists;
//     //         k_proc_thread_lists = thread_list;
//     //     }
//     //     else
//     //     {
//     //         thread_list->next_thread->prev = thread;
//     //     }

//     //     thread->next = thread_list->next_thread;
//     //     thread_list->next_thread = thread;
//     // }
// }

struct k_proc_thread_t *k_proc_CreateThread(void (*thread_fn)(), uint32_t privilege_level)
{
    // uint32_t thread_id = dg_StackListAllocElement(&k_proc_threads);
    // struct k_proc_thread_t *thread = dg_StackListGetElement(&k_proc_threads, thread_id);
    // struct k_proc_thread_t *thread = k_proc_AllocThread();

    uint32_t thread_id = k_mem_AllocObjListElement(&k_proc_threads);
    struct k_proc_thread_t *thread = k_mem_GetObjListElement(&k_proc_threads, thread_id);
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();

    // k_printf("%x\n", thread);
    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();

    thread->tid = thread_id;
    
    // uintptr_t thread_page = (uintptr_t)(thread) & 0xfffff000;
    // union k_proc_thread_list_t *thread_list = (union k_proc_thread_list_t *)thread_page;
    
    // thread->tid = ~((thread_page >> 8) | (thread - thread_list->threads));
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

    current_process->last_thread = thread;

    // uintptr_t stack = (uintptr_t)dg_Malloc(K_PROC_THREAD_STACK_PAGE_COUNT * 0x1000, 4);
    uintptr_t stack = (uintptr_t)k_mem_BigAlloc(&k_proc_kernel_process.heap, K_PROC_THREAD_STACK_PAGE_COUNT * 0x1000, 4);
    thread->stack_base = stack;
    thread->entry_point = (uintptr_t)thread_fn;

    stack += 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT;

    thread->start_esp = (uintptr_t *)stack;
    thread->current_esp = thread->start_esp;
    thread->state = K_PROC_THREAD_STATE_READY;
    thread->page_dir = current_process->page_map.pdir_page;

    uintptr_t start_ebp = (uintptr_t)thread->start_esp;
    uint32_t cs = K_CPU_SEG_SEL(2, 0, 0);
    uint32_t ss = K_CPU_SEG_SEL(1, 0, 0);
    uint32_t ds = K_CPU_SEG_SEL(1, 0, 0);

    if(privilege_level)
    {
        /* threads not in ring 0 will have a dedicated "stack" to store its context,
        while threads in ring 0 will store its context at the very end of their work stack */

        uintptr_t state_block = (uintptr_t)dg_Malloc(1024, 4);
        state_block += 1024;
        thread->current_esp = (uintptr_t *)state_block;

        cs = K_CPU_SEG_SEL(4, 3, 0);
        ss = K_CPU_SEG_SEL(3, 3, 0);
        ds = K_CPU_SEG_SEL(3, 3, 0);

        /* ss */
        thread->current_esp--;
        *thread->current_esp = ss;
        /* esp */
        thread->current_esp--;
        *thread->current_esp = (uintptr_t )thread->start_esp;

        thread->start_esp = (uintptr_t *)state_block;
    }

    thread->code_seg = cs;
    /* eflags */
    thread->current_esp--;
    *thread->current_esp = K_CPU_STATUS_REG_INIT_VALUE | K_CPU_STATUS_FLAG_INT_ENABLE;
    /* cs */
    thread->current_esp--;
    *thread->current_esp = cs;
    /* eip */
    thread->current_esp--;
    *thread->current_esp = thread->entry_point;
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

    // thread->next = k_proc_threads;
    // if(k_proc_threads)
    // {
    //     k_proc_threads->prev = thread;
    // }
    // k_proc_threads = thread;

    return thread;
}

struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id)
{
    struct k_proc_thread_t *thread = NULL;
    thread = k_mem_GetObjListElement(&k_proc_threads, thread_id);

    if(thread && thread->tid == 0xffffffff)
    {
        thread = NULL;
    }

    return thread;
}

struct k_proc_thread_t *k_proc_GetCurrentThread()
{
    return k_proc_current_thread;
}

void k_proc_RunThread(struct k_proc_thread_t *thread)
{
    k_apic_StartTimer(0x1fff);
    thread->state = K_PROC_THREAD_STATE_RUNNING;
    k_proc_SwitchToThread(thread);
    thread->state = K_PROC_THREAD_STATE_READY;
    k_apic_StopTimer();
    k_apic_EndOfInterrupt();
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

void k_proc_RunScheduler()
{
    uint32_t thread_index = 0;
    k_proc_current_thread = &k_proc_scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_Yield();

    // struct k_proc_thread_t *thread = NULL;

    while(1)
    {
        if(k_proc_threads.cursor)
        {
            uint32_t thread_id = (thread_index + k_rng_Rand()) % k_proc_threads.cursor;
            // uint32_t thread_id = (thread_index + 1) % k_proc_threads.cursor; 
            // thread_index = thread_id;
            struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);
            // k_printf("\n schedule thread %d\n", thread->tid);
            if(thread->state == K_PROC_THREAD_STATE_READY)
            {
                k_proc_RunThread(thread);
            }

            thread = thread->next;
        }
    }
}

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}

void k_proc_EnablePreemption()
{
    k_cpu_EnableInterrupts();
    
    if(k_apic_ReadReg(K_PROC_THREAD_PREEMPT_ISR_REG) & K_PROC_THREAD_PREEMPT_ISR_BIT)
    {
        k_proc_Yield();
    }
}

void k_proc_DisablePreemption()
{
    k_cpu_DisableInterrupts();
}

void func1()
{
    uint32_t value = 0;  
    uint32_t old = 0;
    // uint32_t preempt_disabled = 0;
    
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    while(1)
    {
        if(dg_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d -- new: %x, old: %x      \n", current_thread->tid, value, old);
            old = dg_Inc32Wrap(&value);

            // if(old >= 0x3fff && old <= 0xffff0000)
            // {
            //     if(!preempt_disabled)
            //     {
            //         preempt_disabled = 1;
            //         k_proc_DisablePreemption();
            //     }
            // }
            // else
            // {
            //     if(preempt_disabled)
            //     {
            //         k_proc_EnablePreemption();
            //         preempt_disabled = 0;
            //     }
            // }
            // k_atm_Dec32Clamp(&value, 0xffff, &old);
            // value ^= 0x03030303;
            dg_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_proc_Yield();
        }
    }
}

void func2()
{
    uint32_t value = 0;

    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    while(1) 
    {
        if(dg_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x         \n", current_thread->tid, value);
            value++;
            dg_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_proc_Yield();
        }
    }
}

void func3()
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    while(1) 
    {
        if(dg_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: blah        \n", current_thread->tid);
            dg_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_proc_Yield();
        }
    }
}

void func4()
{
    uint32_t value = 0x0000ffff;
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    // k_proc_CreateThread(func5, 0);

    while(1)
    {
        if(dg_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x        \n", current_thread->tid, value);
            value = k_rng_Rand();
            dg_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_proc_Yield();
        }
    }
}

void func5()
{
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    while(1)
    {
        k_printf("\rhello from thread %d, created from another thread!:\n", current_thread->tid);
    }
}

void func6()
{
    uint32_t value = 1;
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    while(1)
    {
        k_printf("\rhello from thread %d: %x        ", current_thread->tid, value);
        value ^= 1;
    }
}

void func7()
{

}
