#include <stddef.h>
#include "k_proc.h"
#include "k_term.h"
#include "k_apic.h"
#include "k_rng.h"
#include "k_atm.h"
#include "k_cpu.h"
#include "mem/k_mem.h"

uint32_t k_proc_process_count = 0;
uint32_t k_proc_pid = 0;
struct k_proc_state_t *k_proc_processes = NULL;

uint32_t k_thread_count = 0;
struct k_proc_thrd_t k_threads[16];
struct k_proc_thrd_t k_scheduler_thread;
struct k_proc_thrd_t *k_active_thread = NULL;
struct k_proc_thrd_t *k_next_thread = NULL;

k_atm_spnl_t k_proc_spinlock = 0;


// struct k_thread_stack_t
// {
//     uint8_t stack[1024];

// }k_thread_stacks[2];

#define K_PROC_THREAD_STACK_SIZE 4096
extern struct k_mem_range_t *k_mem_ranges;
extern uint32_t k_mem_range_count;


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
    struct k_mem_range_t *last_range = k_mem_ranges + (k_mem_range_count - 1);
    uint8_t *stack = (uint8_t *)((uint32_t)last_range->base + k_thread_count * K_PROC_THREAD_STACK_SIZE);

    k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, (uint32_t)stack, (uint32_t)stack, K_MEM_PENTRY_FLAG_READ_WRITE);

    thread->tid = k_thread_count;
    thread->eax = 0;
    thread->ebx = 0;
    thread->ecx = 0;
    thread->edx = 0;
    thread->edi = 0;
    thread->esi = 0;
    thread->cs = 0x0010;
    thread->eif = K_CPU_STATUS_REG_INIT_VALUE;
    thread->eif |= K_CPU_STATUS_FLAG_INT_ENABLE;
    
    thread->ebp = (uint32_t)(stack + K_PROC_THREAD_STACK_SIZE - 12);
    thread->esp = thread->ebp;
    thread->eip = (uint32_t)thread_fn;

    k_thread_count++;

    return thread;
}

void func1()
{
    uint32_t value = 0x01010101;  
    uint32_t direction = 1;
    while(1)
    {
        if(k_atm_TrySpinLock(&k_proc_spinlock))
        {
            k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, value);

            if(direction)
            {
                value <<= 1;
                direction = 0;
            }
            else
            {
                value >>= 1;
                direction = 1;
            }

            k_atm_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_printf("\rhello from thread %d: locked!        ", k_active_thread->tid);
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
            k_printf("\rhello from thread %d: %x        ", k_active_thread->tid, value);
            value = k_rng_Rand();

            k_atm_SpinUnlock(&k_proc_spinlock);
        }
        else
        {
            k_printf("\rhello from thread %d: locked!        ", k_active_thread->tid);
        }
    }
}

void func3()
{
    while(1) 
    {
        k_atm_SpinLock(&k_proc_spinlock);
        k_printf("\rhello from thread %d: blah        ", k_active_thread->tid);
        k_atm_SpinUnlock(&k_proc_spinlock);
    }
}

void k_proc_RunScheduler()
{
    uint32_t thread_index = 0;

    k_active_thread = &k_scheduler_thread;

    while(1)
    {
        if(k_thread_count)
        {
            // uint32_t next_thread = thread_index;
            // thread_index = (thread_index + 1) % k_thread_count;

            uint32_t next_thread = k_rng_Rand() % k_thread_count;

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
            k_apic_StartTimer(0x1ffff);
            k_proc_SwitchToThread(thread);
            k_apic_SignalFixedInterruptHandled();
        }
    }
}

void k_proc_Yield()
{
    k_proc_SwitchToThread(NULL);
}