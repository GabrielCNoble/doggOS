#include <stddef.h>
#include "k_proc.h"
#include "../k_term.h"
#include "../k_apic.h"
#include "../k_rng.h"
#include "../mem/k_mem.h"
#include "../../libdg/atomic/dg_atomic.h"
#include "../../libdg/container/dg_slist.h"
#include "../../libdg/malloc/dg_malloc.h"

struct dg_slist_t k_proc_processes;
struct dg_slist_t k_proc_threads;
struct k_proc_process_t *k_proc_current_process = NULL;

struct k_proc_process_t k_proc_kernel_process;
struct k_proc_thread_t k_proc_scheduler_thread;
struct k_proc_thread_t *k_proc_current_thread = NULL;
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
 
void k_proc_Init()
{
    uint32_t tss_page = k_mem_AllocPage(0);
    k_proc_gdt[5] = K_CPU_SEG_DESC(tss_page, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_mem_MapAddress(tss_page, tss_page, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    k_cpu_Lgdt(k_proc_gdt, 7, K_CPU_SEG_SEL(2, 0, 0));
    k_proc_tss = (struct k_cpu_tss_t *)tss_page;
    k_proc_tss->ss0 = K_CPU_SEG_SEL(1, 0, 0);
    k_cpu_Ltr(K_CPU_SEG_SEL(5, 3, 0));

    k_proc_current_process = &k_proc_kernel_process;
    k_proc_current_thread = &k_proc_scheduler_thread;
    k_proc_scheduler_thread.process = &k_proc_kernel_process;

    for(uint32_t bucket_index = 0; bucket_index < K_MEM_SMALL_BUCKET_COUNT; bucket_index++)
    {
        k_proc_scheduler_thread.heap.buckets[bucket_index].first_chunk = NULL;
        k_proc_scheduler_thread.heap.buckets[bucket_index].last_chunk = NULL;
    }

    k_proc_kernel_process.pid = K_PROC_KERNEL_PID;
    k_proc_processes = dg_StackListCreate(sizeof(struct k_proc_process_t), 512);
    k_proc_threads = dg_StackListCreate(sizeof(struct k_proc_thread_t), 512);
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
    struct k_proc_process_t *process = NULL;

    process = dg_StackListGetElement(&k_proc_processes, process_id);

    if(process && process->pid == DG_INVALID_INDEX)
    {
        process = NULL;
    }

    return process;
}

struct k_proc_process_t *k_proc_GetCurrentProcess()
{
    return k_proc_current_process;
}

uint32_t k_proc_CreateThread(void (*thread_fn)(), uint32_t privilege_level)
{
    uint32_t thread_id = dg_StackListAllocElement(&k_proc_threads);
    struct k_proc_thread_t *thread = dg_StackListGetElement(&k_proc_threads, thread_id);
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();

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

    current_process->last_thread = thread;

    uint8_t *stack = dg_Malloc(K_PROC_THREAD_STACK_PAGE_COUNT * 0x1000, 4);
    thread->stack_base = stack;
    thread->entry_point = thread_fn;

    stack += 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT;

    thread->start_esp = (uint32_t *)stack;
    thread->current_esp = thread->start_esp;
    thread->state = K_PROC_THREAD_STATE_READY;

    uint32_t start_ebp = (uint32_t)thread->start_esp;
    uint32_t cs = K_CPU_SEG_SEL(2, 0, 0);
    uint32_t ss = K_CPU_SEG_SEL(1, 0, 0);
    uint32_t ds = K_CPU_SEG_SEL(1, 0, 0);

    if(privilege_level)
    {
        /* threads not in ring 0 will have a dedicated "stack" to store its context,
        while threads in ring 0 will store its context at the very end of their work stack */

        uint8_t *state_block = dg_Malloc(1024, 4);
        // k_printf("pl 3 state block at %x\n", state_block);
        // k_cpu_Halt();
        state_block += 1024;
        thread->current_esp = (uint32_t *)state_block;

        cs = K_CPU_SEG_SEL(4, 3, 0);
        ss = K_CPU_SEG_SEL(3, 3, 0);
        ds = K_CPU_SEG_SEL(3, 3, 0);

        /* ss */
        thread->current_esp--;
        *thread->current_esp = ss;
        /* esp */
        thread->current_esp--;
        *thread->current_esp = (uint32_t)thread->start_esp;

        thread->start_esp = (uint32_t *)state_block;
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
    *thread->current_esp = (uint32_t)thread->entry_point;
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
    /* cr3 */
    thread->current_esp--;
    *thread->current_esp = k_cpu_Rcr3();
    /* ds */
    thread->current_esp--;
    *thread->current_esp = ds;
    /* es */
    thread->current_esp--;
    *thread->current_esp = ds;
    /* fs */
    thread->current_esp--;
    *thread->current_esp = ds;

    return thread->tid;
}

struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id)
{
    struct k_proc_thread_t *thread = NULL;
    thread = dg_StackListGetElement(&k_proc_threads, thread_id);

    if(thread && thread->tid == DG_INVALID_INDEX)
    {
        thread = NULL;
    }

    return thread;
}

struct k_proc_thread_t *k_proc_GetCurrentThread()
{
    return k_proc_current_thread;
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

}

void k_proc_RunScheduler()
{
    uint32_t thread_index = 0;
    k_proc_current_thread = &k_proc_scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_Yield();

    while(1)
    {
        if(k_proc_threads.cursor)
        {
            uint32_t thread_id = (thread_index + k_rng_Rand()) % k_proc_threads.cursor;
            thread_index = thread_id;
            struct k_proc_thread_t *thread = k_proc_GetThread(thread_id);
            if(thread->state == K_PROC_THREAD_STATE_READY)
            {
                k_apic_StartTimer(0xff);
                thread->state = K_PROC_THREAD_STATE_RUNNING;
                k_proc_SwitchToThread(thread);
                thread->state = K_PROC_THREAD_STATE_READY;
                k_apic_StopTimer();
                k_apic_EndOfInterrupt();
            }
        }
    }
}

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}

void func1()
{
    uint32_t value = 0xffff;  
    uint32_t old = 0;
    
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();

    while(1)
    {
        if(dg_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d -- new: %x, old: %x      \n", current_thread->tid, value, old);
            old = dg_Dec32Wrap(&value);
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
    while(1)
    {
        // k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, k_active_thread->tss.eax);
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
