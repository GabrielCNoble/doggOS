#include "../../kernel/rt/alloc.h"
#include "../../kernel/rt/atm.h"
#include "../../kernel/dsk/dsk.h"
#include "../../kernel/proc/thread.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* in order to use the kernel file system code in the tool, we need to stub out
some of its functions here. */

void *k_rt_Malloc(size_t size, size_t align)
{
    size_t alloc_size = (size + 4095) & (~4095);

    if(size % 4096)
    {
        alloc_size += 4096;
    }

    return malloc(alloc_size);
}

void k_rt_Free(void *memory)
{
    free(memory);
}

uint32_t k_rt_Inc32Wrap(uint32_t *location)
{
    return 1;
}

uint32_t k_rt_Dec32Wrap(uint32_t *location)
{
    return 1;
}

void k_rt_SpinLock(k_rt_spnl_t *lock)
{
    *lock = 1;
}

void k_rt_SpinLockCritical(k_rt_spnl_t *lock)
{
    k_rt_SpinLock(lock);
}

uint32_t k_rt_TrySpinLock(k_rt_spnl_t *lock)
{
    *lock = 1;
    return 1;
}

void k_rt_SpinUnlock(k_rt_spnl_t *lock)
{
    *lock = 0;
}

void k_rt_SpinUnlockCritical(k_rt_spnl_t *lock)
{
    k_rt_SpinUnlock(lock);
}

uint32_t k_rt_CmpXchg(uintptr_t *location, uintptr_t cmp, uintptr_t new, uintptr_t *old)
{
    if(*location == cmp)
    {
        if(old != NULL)
        {
            *old = *location;
        }

        *location = new;
        return 1;
    }

    return 0;
}

uint32_t k_rt_CmpXchg32(uint32_t *location, uint32_t cmp, uint32_t new, uint32_t *old)
{
    if(*location == cmp)
    {
        if(old != NULL)
        {
            *old = *location;
        }

        *location = new;
        return 1;
    }

    return 0;
}

uint32_t k_rt_CmpXchg8(uint8_t *location, uint8_t cmp, uint8_t new, uint8_t *old)
{
    if(*location == cmp)
    {
        if(old != NULL)
        {
            *old = *location;
        }

        *location = new;
        return 1;
    }

    return 0;
}

uint32_t k_rt_AtomicOr32(uint32_t *location, uint32_t operand)
{
    uint32_t old = *location;
    *location |= operand;
    return old;
}

uint32_t k_rt_AtomicAnd32(uint32_t *location, uint32_t operand)
{
    uint32_t old = *location;
    *location &= operand;
    return old;
}

void k_rt_SignalCondition(k_rt_cond_t *condition)
{
    *condition = 1;
}

uint32_t k_rt_SpinWait(k_rt_spnl_t *lock)
{
    return 1;
}

void k_rt_CopyBytes(void * restrict dst, const void * restrict src, size_t size)
{
    memcpy(dst, src, size);
}

void k_rt_SetBytes(void * restrict dst, size_t size, uint32_t value)
{
    memset(dst, value, size);
}

int32_t k_rt_StrCmp(const char *str0, const char *str1)
{
    return strcmp(str0, str1);
}

uint32_t k_rt_StrCpy(char *buffer, uint32_t buffer_size, const char *str)
{
    strncpy(buffer, str, buffer_size);
    return strlen(buffer) + 1;
}

uint32_t k_rt_StrLen(const char *str)
{
    return strlen(str);
}

uint32_t k_rt_StrCat(char *buffer, uint32_t buffer_size, const char *str)
{
    strncat(buffer, str, buffer_size);
    return strlen(buffer) + 1;
}

struct k_proc_thread_t *k_proc_CreateKernelThread(k_proc_thread_func_t entry_point, void *user_data)
{
    return NULL;
}

void k_proc_YieldThread()
{

}

uint32_t k_proc_WaitCondition(k_rt_cond_t *condition)
{
    return 0;
}

void k_cpu_MemFence()
{
    
}

void k_sys_TerminalPrintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void k_sys_RaiseException(uint32_t exception)
{

}



void k_dev_DeviceReady(struct k_dev_device_t *device)
{

}
// uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
// {

// }

// uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
// {

// }