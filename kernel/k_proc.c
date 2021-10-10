#include <stddef.h>
#include "k_proc.h"
#include "k_term.h"
#include "k_apic.h"
#include "k_rng.h"
#include "k_atm.h"
#include "mem/k_mem.h"

uint32_t k_proc_process_count = 0;
uint32_t k_proc_pid = 0;
struct k_proc_state_t *k_proc_processes = NULL;

struct k_proc_thrd_t *k_running_threads[16];
uint32_t k_runnning_count = 0;

struct k_proc_thrd_t *k_queued_threads[16];
k_atm_spnl_t k_queue_spinlock = 0;
uint32_t k_queue_next_in = 0;
uint32_t k_queue_next_out = 0;

struct k_proc_thrd_t k_threads[16];
uint32_t k_thread_count = 0;

struct k_proc_thrd_t k_scheduler_thread;
struct k_proc_thrd_t *k_active_thread = NULL;
struct k_proc_thrd_t *k_next_thread = NULL;

k_atm_spnl_t k_proc_spinlock = 0;


// struct k_thread_stack_t
// {
//     uint8_t stack[1024];

// }k_thread_stacks[2];

#define K_PROC_THREAD_STACK_PAGE_COUNT 4
// #define K_PROC_THREAD_STACK_SIZE 0x1000
// extern struct k_mem_range_t *k_mem_ranges;
// extern uint32_t k_mem_range_count;

// uint32_t k_proc_src_block;
// uint32_t k_proc_src_block_cursor = 0;

struct k_cpu_tss_t *k_proc_tss;
struct k_cpu_seg_desc_t k_proc_gdt[] = 
{ 
    K_CPU_SEG_DESC(0x00000000u, 0x000000u, 0, 0, 0, 0, 0),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1), 
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    K_CPU_SEG_DESC(0x00000000u, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1),
}; 
 
void k_proc_Init()
{
    k_proc_tss = (struct k_cpu_tss_t *)k_mem_AllocPage(0);   
    k_proc_gdt[5] = K_CPU_SEG_DESC((uint32_t)k_proc_tss, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, (uint32_t)k_proc_tss, (uint32_t)k_proc_tss, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);

    uint32_t stack = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
    k_proc_tss->ss0 = 0x0008;
    k_proc_tss->esp0 = stack + 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT - 24;
    for(uint32_t index = 0; index < K_PROC_THREAD_STACK_PAGE_COUNT; index++)
    {
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, stack + 0x1000 * index, stack + 0x1000 * index, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    }

    stack = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
    k_proc_tss->ss1 = 0x0008;
    k_proc_tss->esp1 = stack + 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT - 24;
    for(uint32_t index = 0; index < K_PROC_THREAD_STACK_PAGE_COUNT; index++)
    {
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, stack + 0x1000 * index, stack + 0x1000 * index, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    }

    stack = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
    k_proc_tss->ss2 = 0x0008;
    k_proc_tss->esp2 = stack + 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT - 24;
    for(uint32_t index = 0; index < K_PROC_THREAD_STACK_PAGE_COUNT; index++)
    {
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, stack + 0x1000 * index, stack + 0x1000 * index, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    }

    // k_proc_tss->ss = 0x0008;
    // k_proc_tss->esp = 0x0000;

    k_printf("esp0 at: %x\n", k_proc_tss->esp0);
    k_cpu_Lgdt(k_proc_gdt, 6);
    k_cpu_Ltr(0x28);
}

uint32_t k_proc_CreateProcess(struct k_mem_pstate_t *pstate, uint32_t start_address, void *image, uint32_t size)
{
    // uint8_t *process_buffer = k_mem_alloc(size + sizeof(struct k_proc_state_t), sizeof(struct k_proc_state_t));
    // struct k_proc_state_t *proc_state = (struct k_proc_state_t *)process_buffer;
    // process_buffer += sizeof(struct k_proc_state_t);

    // proc_state->pstate = k_mem_create_pstate();
    // proc_state->pid = 0;
}

struct k_proc_thrd_t *k_proc_CreateThread(void (*thread_fn)())
{
    struct k_proc_thrd_t *thread = k_threads + k_thread_count;
    uint32_t stack = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
    

    for(uint32_t page_index = 0; page_index < K_PROC_THREAD_STACK_PAGE_COUNT; page_index++)
    {
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, (uint32_t)stack + 0x1000 * page_index, (uint32_t)stack + 0x1000 * page_index, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    }

    thread->tid = k_thread_count;
    thread->tss.eax = 0;
    thread->tss.ebx = 0;
    thread->tss.ecx = 0;
    thread->tss.edx = 0;
    thread->tss.edi = 0;
    thread->tss.esi = 0;

    thread->tss.cs = 0x0023;
    thread->tss.ds = 0x001b;
    thread->tss.ss = 0x001b;
    thread->tss.es = 0x001b;
    thread->tss.fs = 0x001b;
    thread->tss.gs = 0x0000;

    // thread->tss.cs = 0x0010;
    // thread->tss.ds = 0x008;
    // thread->tss.ss = 0x008;
    // thread->tss.es = 0x008;
    // thread->tss.fs = 0x008;
    // thread->tss.gs = 0x008;
    thread->tss.eflags = K_CPU_STATUS_REG_INIT_VALUE | K_CPU_STATUS_FLAG_INT_ENABLE;
    
    thread->tss.ebp = stack + 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT - 12;
    thread->tss.esp = thread->tss.ebp;
    thread->tss.eip = (uint32_t)thread_fn;

    thread->tss.cr3 = k_cpu_Rcr3(); 

    k_thread_count++;

    k_printf("thread %d stack at %x\n", thread->tid, thread->tss.esp);

    return thread;
}

void k_proc_QueueThread(struct k_proc_thrd_t *thread)
{

}

void func1()
{
    uint32_t value = 0x01010101;  
    uint32_t direction = 1;
    while(1)
    {
        // k_atm_SpinLock(&k_proc_spinlock);
        k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, value);
        value ^= 0x03030303;
        // if(direction)
        // {
        //     value <<= 1;
        //     direction = 0;
        // }
        // else
        // {
        //     value >>= 1;
        //     direction = 1;
        // }

        // for(uint32_t i = 0; i < 0x1fff; i++);
        // k_atm_SpinUnlock(&k_proc_spinlock);
    }
}

void func2()
{
    uint32_t value = 0;
    while(1) 
    {
        k_printf("\rhello from thread %d: %x         ", k_active_thread->tid, value);
        value++;
    }
}

void func3()
{
    while(1) 
    {
        // k_atm_SpinLock(&k_proc_spinlock);
        k_printf("\rhello from thread %d: blah        ", k_active_thread->tid);
        // k_atm_SpinUnlock(&k_proc_spinlock);
    }
}

void func4()
{
    uint32_t value = 0x0000ffff;
    while(1)
    {
        // k_atm_SpinLock(&k_proc_spinlock);
        k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, value);
        value = k_rng_Rand();
        // k_atm_SpinUnlock(&k_proc_spinlock);
    }
}

void func5()
{
    while(1)
    {
        k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, k_active_thread->tss.eax);
    }
}

void func6()
{
    uint32_t value = 1;
    while(1)
    {
        k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, value);
        value ^= 1;
    }
}

void func7()
{

}

void k_proc_RunScheduler()
{
    uint32_t thread_index = 0;
    k_active_thread = &k_scheduler_thread;
    // k_proc_RunThread(NULL);

    // k_proc_tss->esp0 = k_scheduler_thread.tss.esp;
    // k_proc_tss->ss0 = k_scheduler_thread.tss.ss;

    // k_printf("scheduler: %x %x\n", k_scheduler_thread.tss.esp0, (uint32_t)k_scheduler_thread.tss.ss0);

    while(1)
    {
        if(k_thread_count)
        {
            uint32_t next_thread = (thread_index + k_rng_Rand()) % k_thread_count;
            thread_index = next_thread;
            struct k_proc_thrd_t *thread = k_threads + next_thread;
            k_printf("\n");
            asm volatile 
            (
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
            );
            k_apic_StartTimer(0x7f);
            k_proc_RunThread(thread);
            k_apic_SignalFixedInterruptHandled();
        }
    }
}

void k_proc_Yield()
{
    k_proc_RunThread(NULL);
}