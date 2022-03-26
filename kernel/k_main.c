#include "k_main.h"
// #include "../libdg/atomic/dg_atomic.h"
// #include "../libdg/malloc/dg_malloc.h"
// #include "../libdg/string/dg_string.h"
#include "rt/atm.h"
#include "proc/thread.h"
#include "proc/proc.h"
#include "k_rng.h"
#include "rt/mem.h"
#include "rt/queue.h"
#include "../version/version.h"
// #include "shell.h"

extern void *k_int8_a;

struct k_test_buffer_t
{
    uint32_t start;
    uint32_t end;
    uint32_t *buffer;
};

uintptr_t k_min_value(void *data)
{
    // struct k_test_buffer_t *in_buffer = (struct k_test_buffer_t *)data;
    // uint32_t size = (in_buffer->end - in_buffer->start) + 1;

    // if(size > 2)
    // {
    //     uint32_t middle = (in_buffer->end + in_buffer->start) / 2;
    //     struct k_test_buffer_t out_buffer = {};
    //     // struct k_proc_thread_t *thread;
    //     uintptr_t value_a;
    //     uintptr_t value_b;

    //     out_buffer.start = in_buffer->start;
    //     out_buffer.end = middle - 1;
    //     out_buffer.buffer = in_buffer->buffer;
    //     uint32_t thread = k_proc_CreateThread(k_min_value, &out_buffer);
    //     k_proc_WaitThread(thread, &value_a);

    //     out_buffer.start = middle;
    //     out_buffer.end = in_buffer->end;
    //     thread = k_proc_CreateThread(k_min_value, &out_buffer);
    //     k_proc_WaitThread(thread, &value_b);

    //     if(value_a < value_b)
    //     {
    //         return value_a;
    //     }

    //     return value_b;
    // }
    // else if(size == 2)
    // {
    //     if(in_buffer->buffer[in_buffer->start] < in_buffer->buffer[in_buffer->end])
    //     {
    //         return (uintptr_t)in_buffer->buffer[in_buffer->start];
    //     }
    //     else
    //     {
    //         return (uintptr_t)in_buffer->buffer[in_buffer->end];
    //     }
    // }

    // return (uintptr_t)in_buffer->buffer[in_buffer->start];
}

uintptr_t k_max_value(void *data)
{
    // struct k_test_buffer_t *in_buffer = (struct k_test_buffer_t *)data;
    // uint32_t size = (in_buffer->end - in_buffer->start) + 1;

    // if(size > 2)
    // {
    //     uint32_t middle = (in_buffer->end + in_buffer->start) / 2;
    //     struct k_test_buffer_t out_buffer = {};
    //     struct k_proc_thread_t *thread;
    //     uintptr_t value_a;
    //     uintptr_t value_b;

    //     out_buffer.start = in_buffer->start;
    //     out_buffer.end = middle - 1;
    //     out_buffer.buffer = in_buffer->buffer;
    //     thread = k_proc_CreateThread(k_max_value, &out_buffer);
    //     k_proc_WaitThread(thread->tid, &value_a);

    //     out_buffer.start = middle;
    //     out_buffer.end = in_buffer->end;
    //     thread = k_proc_CreateThread(k_max_value, &out_buffer);
    //     k_proc_WaitThread(thread->tid, &value_b);

    //     if(value_a > value_b)
    //     {
    //         return value_a;
    //     }

    //     return value_b;
    // }
    // else if(size == 2)
    // {
    //     if(in_buffer->buffer[in_buffer->start] > in_buffer->buffer[in_buffer->end])
    //     {
    //         return (uintptr_t)in_buffer->buffer[in_buffer->start];
    //     }
    //     else
    //     {
    //         return (uintptr_t)in_buffer->buffer[in_buffer->end];
    //     }
    // }

    // return (uintptr_t)in_buffer->buffer[in_buffer->start];
}

uintptr_t k_merge_sort(void *data)
{
    // struct k_test_buffer_t *in_buffer = (struct k_test_buffer_t *)data;
    // uint32_t size = (in_buffer->end - in_buffer->start) + 1;

    // if(size > 2)
    // {
    //     uint32_t middle = (in_buffer->end + in_buffer->start) / 2;

    //     uint32_t buffer_a[size / 2];
    //     struct k_test_buffer_t out_buffer_a = {.buffer = buffer_a, .start = in_buffer->start, .end = middle - 1};
    //     uint32_t buffer_b[size / 2];
    //     struct k_test_buffer_t out_buffer_b = {.buffer = buffer_b, .start = middle, .end = in_buffer->end};

    //     k_util_CopyBytes(out_buffer_a.buffer, in_buffer->buffer, sizeof(uint32_t) * (out_buffer_a->end - out_buffer_a->start + 1));
    //     k_util_CopyBytes(out_buffer_b.buffer, in_buffer->buffer + out_buffer_b->start, sizeof(uint32_t) * (out_buffer_b->end - out_buffer_b->start + 1));

    //     struct k_proc_thread_t *thread_a = k_proc_CreateThread(k_merge_sort, &out_buffer_a);
    // }
    // else if(size == 2)
    // {
    //     if(in_buffer->buffer[in_buffer->start] > in_buffer->buffer[in_buffer->end])
    //     {
    //         uint32_t temp = in_buffer->buffer[in_buffer->start];
    //         in_buffer->buffer[in_buffer->start] = in_buffer->buffer[in_buffer->end];
    //         in_buffer->buffer[in_buffer->end] = temp;
    //     }
    // }

    // return 0;
}

uintptr_t k_thread_tests(void *data)
{
    // uint32_t start_buffer[8];
    // uint32_t test_buffer[8];
    // uintptr_t result;

    // struct k_test_buffer_t buffer = {};
    // buffer.buffer = test_buffer;
    // buffer.start = 0;
    // buffer.end = (sizeof(test_buffer) / sizeof(test_buffer[0])) - 1;

    // for(uint32_t index = 0; index < sizeof(start_buffer) / sizeof(start_buffer[0]); index++)
    // {
    //     start_buffer[index] = k_rng_Rand() % 128;
    // }
    // k_util_CopyBytes(buffer.buffer, start_buffer, sizeof(start_buffer));

    // k_printf("input buffer\n");
    // for(uint32_t index = 0; index < sizeof(start_buffer) / sizeof(start_buffer[0]); index++)
    // {
    //     k_printf("%d ", start_buffer[index]);
    // }
    // k_printf("\n");

    // uint32_t min_value_thread = k_proc_CreateThread(k_min_value, &buffer);
    // k_proc_WaitThread(min_value_thread, &result);
    // k_printf("min value is: %d\n", (uint32_t)result);

    // struct k_proc_thread_t *max_value_thread = k_proc_CreateThread(k_max_value, &buffer);
    // k_proc_WaitThread(max_value_thread->tid, &result);
    // k_printf("max value is: %d\n", (uint32_t)result);

    return 0;
}

k_rt_spnl_t spinlock = 0;

uintptr_t simple_test1(void *data)
{
    // uint32_t value = 0;
    // struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    // while(1)
    // {
    //     if(k_atm_TrySpinLock(&spinlock))
    //     {
    //         k_printf("\rthread %x: %x             ", current_thread, value);
    //         value++;
    //         k_atm_SpinUnlock(&spinlock);
    //         k_proc_Yield();
    //     }
    // }
    while(1)
    {
        k_sys_TerminalPrintf("b\n");
    }
}

uintptr_t simple_test2(void *data)
{
    // uint32_t value = 0xffffffff;
    // struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    // while(1)
    // {
    //     if(k_atm_TrySpinLock(&spinlock))
    //     {
    //         k_printf("\rthread %x: %x             ", current_thread, value);
    //         value--;
    //         k_atm_SpinUnlock(&spinlock);
    //         k_proc_Yield();
    //     }
    // }
    while(1)
    {
        k_sys_TerminalPrintf("a\n");
    }
}

uintptr_t work_thread_test(void *data)
{
    uint32_t value = 0;
    uint32_t return_value;
    struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    while(value < 0xff)
    {
        return_value = k_rng_Rand() % 330;
        value++;
    }

    // return_value = 0x0b00b1e5;

    // k_printf("\nfinal value of thread %x is %x                   \n", current_thread, return_value);

    return (uintptr_t)return_value;
}

uintptr_t waiting_thread_test(void *data)
{
    // uintptr_t return_value;
    // struct k_proc_thread_t *current_thread = k_proc_GetCurrentThread();
    // struct k_proc_thread_t *work_thread = k_proc_CreateThread(work_thread_test, NULL);
    // // k_printf("thread %x waiting on thread %x\n", current_thread, work_thread);
    // // for(uint32_t index = 0; index < 0x1ffff; index++)
    // // {
    // //     return_value = k_rng_Rand() % 55;
    // // }
    // // k_printf("thread %x done working, now check value from thread %x\n", current_thread, work_thread);
    // k_proc_WaitThread(work_thread, &return_value);
    // // for(uint32_t index = 0; index < 0x1ff; index++)
    // // {
    // //     return_value = k_rng_Rand() % 55;
    // // }
    // // k_printf("thread %x returned with value %x\n", work_thread, return_value);
    // // while(1);
    // return return_value;
}

uintptr_t main_thread(void *data)
{
    while(1)
    {
        uintptr_t return_value;
        // k_term_clear();
        // struct k_proc_thread_t *work_thread = k_proc_CreateKernelThread(work_thread_test, NULL);
        // k_proc_WaitThread(work_thread, &return_value);
        // if(k_proc_WaitThread(work_thread, &return_value) != K_STATUS_OK)
        // {
        //     k_printf("%x is detached        \n", work_thread);
        // }
        // else
        // {
        //     k_printf("%x == %x           \n", work_thread, return_value);
        // }
        k_sys_TerminalPrintf("%x\n", k_rng_Rand());
    }
    return 1;
}

extern void *k_kernel_end2;

void k_main()
{
    _Static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "Size of uintptr_t doesn't match size of uint32_t");
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_WHITE, K_SYS_TERM_COLOR_BLACK);

    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();

    struct k_proc_thread_t *shell_thread = k_proc_CreateKernelThread(k_sys_ShellMain, NULL);
    k_proc_RunScheduler();

    return;
}
