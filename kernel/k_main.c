#include "k_main.h"
#include "../libdg/atomic/dg_atomic.h"
#include "../libdg/malloc/dg_malloc.h"
#include "../libdg/string/dg_string.h"
#include "timer/k_apic.h"
#include "proc/k_thread.h"
#include "proc/k_proc.h"
#include "k_rng.h"
#include "k_int.h"
#include "util/k_util.h"
#include "../version/version.h"

extern void *k_int8_a;

struct k_test_buffer_t
{
    uint32_t start;
    uint32_t end;
    uint32_t *buffer;
};

uintptr_t k_min_value(void *data)
{
    struct k_test_buffer_t *in_buffer = (struct k_test_buffer_t *)data;
    uint32_t size = (in_buffer->end - in_buffer->start) + 1;

    if(size > 2)
    {
        uint32_t middle = (in_buffer->end + in_buffer->start) / 2;
        struct k_test_buffer_t out_buffer = {};
        // struct k_proc_thread_t *thread;
        uintptr_t value_a;
        uintptr_t value_b;

        out_buffer.start = in_buffer->start;
        out_buffer.end = middle - 1;
        out_buffer.buffer = in_buffer->buffer;
        uint32_t thread = k_proc_CreateThread(k_min_value, &out_buffer);
        k_proc_WaitThread(thread, &value_a);

        out_buffer.start = middle;
        out_buffer.end = in_buffer->end;
        thread = k_proc_CreateThread(k_min_value, &out_buffer);
        k_proc_WaitThread(thread, &value_b);

        if(value_a < value_b)
        {
            return value_a;
        }

        return value_b;
    }
    else if(size == 2)
    {
        if(in_buffer->buffer[in_buffer->start] < in_buffer->buffer[in_buffer->end])
        {
            return (uintptr_t)in_buffer->buffer[in_buffer->start];
        }
        else
        {
            return (uintptr_t)in_buffer->buffer[in_buffer->end];
        }
    }

    return (uintptr_t)in_buffer->buffer[in_buffer->start];
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
    uint32_t start_buffer[8];
    uint32_t test_buffer[8];
    uintptr_t result;

    struct k_test_buffer_t buffer = {};
    buffer.buffer = test_buffer;
    buffer.start = 0;
    buffer.end = (sizeof(test_buffer) / sizeof(test_buffer[0])) - 1;

    for(uint32_t index = 0; index < sizeof(start_buffer) / sizeof(start_buffer[0]); index++)
    {
        start_buffer[index] = k_rng_Rand() % 128;
    }
    k_util_CopyBytes(buffer.buffer, start_buffer, sizeof(start_buffer));

    k_printf("input buffer\n");
    for(uint32_t index = 0; index < sizeof(start_buffer) / sizeof(start_buffer[0]); index++)
    {
        k_printf("%d ", start_buffer[index]);
    }
    k_printf("\n");

    uint32_t min_value_thread = k_proc_CreateThread(k_min_value, &buffer);
    k_proc_WaitThread(min_value_thread, &result);
    k_printf("min value is: %d\n", (uint32_t)result);

    // struct k_proc_thread_t *max_value_thread = k_proc_CreateThread(k_max_value, &buffer);
    // k_proc_WaitThread(max_value_thread->tid, &result);
    // k_printf("max value is: %d\n", (uint32_t)result);

    return 0;
}

void k_main()
{
    _Static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "Size of uintptr_t doesn't match size of uint32_t");

    k_term_clear();
    k_printf("doggOS version %d.%d.%d+%d\n", K_VERSION_MAJOR, K_VERSION_MINOR, K_VERSION_PATCH, K_VERSION_BUILD);
    k_cpu_DisableInterrupts();
    k_cpu_Halt();

    // uint8_t src[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    // uint8_t dst[sizeof(src) / sizeof(src[0])];

    // asm volatile
    // (
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    // );
    // ds_Memcpy(src + 5, src, 3);
    
    // for(uint32_t index = 0; index < sizeof(src) / sizeof(src[0]); index++)
    // {
    //     k_printf("%d\n", (uint32_t)src[index]);
    // }

    // k_cpu_Halt();

    // k_proc_CreateThread(func1, NULL);
    // k_proc_CreateThread(func2, 0);
    // k_proc_CreateThread(func3, 3);
    // k_proc_CreateThread(func4, NULL);

    // k_proc_CreateThread(k_thread_tests, NULL);
    // k_proc_RunScheduler();
    return;
}