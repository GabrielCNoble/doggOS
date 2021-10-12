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

extern void *k_proc_ThreadSwitchReturnAddress;

// struct k_proc_thrd_t *k_running_threads[16];
// uint32_t k_runnning_count = 0;

// struct k_proc_thrd_t *k_queued_threads[16];
// k_atm_spnl_t k_queue_spinlock = 0;
// uint32_t k_queue_next_in = 0;
// uint32_t k_queue_next_out = 0;

struct k_proc_thread_t k_proc_threads[16]; 
uint32_t k_proc_thread_count = 0;

// struct k_proc_thread_t k_proc_scheduler_thread;
struct k_proc_thread_t k_proc_scheduler_thread;
struct k_proc_thread_t *k_proc_active_thread = NULL;
struct k_proc_thraed_t *k_proc_next_thread = NULL;

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
    K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EOC, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
}; 
 
void k_proc_Init()
{
    k_proc_InitThread(&k_proc_scheduler_thread, NULL, 0);
    uint32_t tss_page = k_mem_AllocPage(0);
    k_proc_tss = (struct k_cpu_tss_t *)tss_page;
    k_proc_gdt[5] = K_CPU_SEG_DESC(tss_page, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);

    // k_proc_gdt[5].v.w.w0 = 0x67;
    // k_proc_gdt[5].v.w.w1 = tss_page & 0xffff;
    // k_proc_gdt[5].v.w.w2 = ((tss_page >> 16) & 0xff) | (K_CPU_SSEG_TYPE_TSS32_AVAL) | (1 << 15);
    // k_proc_gdt[5].v.w.w3 = (tss_page >> 24) & 0xff;

    k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, tss_page, tss_page, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    k_cpu_Lgdt(k_proc_gdt, 7, K_CPU_SEG_SEL(2, 0, 0));
    k_proc_tss->eax = 0x11111111;
    k_proc_tss->ebx = 0x22222222;
    k_proc_tss->ecx = 0x33333333;
    k_proc_tss->edx = 0x44444444;
    k_proc_tss->edi = 0x55555555;
    k_proc_tss->esi = 0x66666666;
    k_proc_tss->ebp = 0x77777777;
    k_proc_tss->esp = 0x88888888;
    k_proc_tss->ss0 = K_CPU_SEG_SEL(1, 0, 0);
    k_proc_tss->ss1 = 0xbaad;
    k_proc_tss->esp1 = 0xbaadf00d;
    k_proc_tss->ss2 = 0xbaad;
    k_proc_tss->esp2 = 0xbaadf00d;
    k_cpu_Ltr(K_CPU_SEG_SEL(5, 3, 0));
    k_printf("tss at: %x -- descriptor value: %x %x %x %x\n", tss_page, (uint32_t)k_proc_gdt[5].v.w.w3, 
                                                                                    (uint32_t)k_proc_gdt[5].v.w.w2, 
                                                                                    (uint32_t)k_proc_gdt[5].v.w.w1,
                                                                                    (uint32_t)k_proc_gdt[5].v.w.w0);
}

uint32_t k_proc_CreateProcess(struct k_mem_pstate_t *pstate, uint32_t start_address, void *image, uint32_t size)
{
    // uint8_t *process_buffer = k_mem_alloc(size + sizeof(struct k_proc_state_t), sizeof(struct k_proc_state_t));
    // struct k_proc_state_t *proc_state = (struct k_proc_state_t *)process_buffer;
    // process_buffer += sizeof(struct k_proc_state_t);

    // proc_state->pstate = k_mem_create_pstate();
    // proc_state->pid = 0;
}

struct k_proc_thread_t *k_proc_CreateThread(void (*thread_fn)(), uint32_t privilege_level)
{
    struct k_proc_thread_t *thread = k_proc_threads + k_proc_thread_count;
    thread->tid = k_proc_thread_count;
    k_proc_InitThread(thread, thread_fn, privilege_level);
    k_proc_thread_count++;

    return thread;
}

void k_proc_InitThread(struct k_proc_thread_t *thread, void (*thread_fn)(), uint32_t privilege_level)
{
    uint32_t stack = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
    for(uint32_t page_index = 0; page_index < K_PROC_THREAD_STACK_PAGE_COUNT; page_index++)
    {
        uint32_t page_address = stack + 0x1000 * page_index;
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, page_address, page_address, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    }
    
    thread->stack_base = stack;
    thread->entry_point = (uint32_t)thread_fn;

    stack += 0x1000 * K_PROC_THREAD_STACK_PAGE_COUNT;

    thread->start_esp = (uint32_t *)stack;
    thread->current_esp = thread->start_esp;

    struct k_proc_tstate_t *state;
    uint32_t start_ebp = thread->start_esp;
    uint32_t cs = K_CPU_SEG_SEL(2, 0, 0);
    uint32_t ss = K_CPU_SEG_SEL(1, 0, 0);
    uint32_t ds = K_CPU_SEG_SEL(1, 0, 0);

    if(privilege_level)
    {
        /* threads not in ring 0 will have a dedicated "stack" to store its context,
        while threads in ring 0 will store its context at the very end of their work stack */
        uint32_t state_page = k_mem_AllocPage(0);
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, state_page, state_page, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);

        // uint32_t state_page = k_mem_AllocPages(K_PROC_THREAD_STACK_PAGE_COUNT, 0);
        // for(uint32_t page_index = 0; page_index < K_PROC_THREAD_STACK_PAGE_COUNT; page_index++)
        // {
        //     uint32_t page_address = state_page + 0x1000 * page_index;
        //     k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, page_address, page_address, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
        // }

        state_page += 0x1000;
        thread->current_esp = (uint32_t *)state_page;

        cs = K_CPU_SEG_SEL(4, 3, 0);
        ss = K_CPU_SEG_SEL(3, 3, 0);
        ds = K_CPU_SEG_SEL(3, 3, 0);

        /* ss */
        thread->current_esp--;
        *thread->current_esp = ss;
        /* esp */
        thread->current_esp--;
        *thread->current_esp = (uint32_t)thread->start_esp;

        thread->start_esp = (uint32_t *)state_page;
    }

    thread->code_seg = cs;
    
    // state = (struct k_proc_tstate_t *)(thread->start_esp - sizeof(struct k_proc_tstate_t) + 1);

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
        if(k_atm_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x        ", k_proc_active_thread->tid, value);
            value ^= 0x03030303;
            k_atm_SpinUnlock(&k_proc_spinlock);
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
    while(1) 
    {
        if(k_atm_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x         ", k_proc_active_thread->tid, value);
            value++;
            k_atm_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_proc_Yield();
        }
    }
}

void func3()
{
    while(1) 
    {
        if(k_atm_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: blah        ", k_proc_active_thread->tid);
            k_atm_SpinUnlock(&k_proc_spinlock);
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
    while(1)
    {
        if(k_atm_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x        ", k_proc_active_thread->tid, value);
            value = k_rng_Rand();
            k_atm_SpinUnlock(&k_proc_spinlock);
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
    while(1)
    {
        k_printf("\rhello from thread %d: %x        ", k_proc_active_thread->tid, value);
        value ^= 1;
    }
}

void func7()
{

}

void k_proc_RunScheduler()
{
    uint32_t thread_index = 0;
    k_proc_active_thread = &k_proc_scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_Yield();

    while(1)
    {
        if(k_proc_thread_count)
        {
            uint32_t next_thread = (thread_index + k_rng_Rand()) % k_proc_thread_count;
            thread_index = next_thread;
            // uint32_t next_thread = thread_index;
            // thread_index = (thread_index + 1) % k_proc_thread_count;
            struct k_proc_thread_t *thread = k_proc_threads + next_thread;
            uint32_t *state = (uint32_t *)thread->current_esp;
            if(k_atm_TrySpinLock(&k_proc_spinlock))
            {
                k_printf("\n"); 
                k_atm_SpinUnlock(&k_proc_spinlock);
            }
            k_apic_StartTimer(0x1fff);
            asm volatile 
            (
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
            );
            k_proc_SwitchToThread(thread);
            k_apic_SignalFixedInterruptHandled();
        }
    }
}

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}