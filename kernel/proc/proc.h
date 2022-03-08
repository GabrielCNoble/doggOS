#ifndef K_PROC_H
#define K_PROC_H

#include "defs.h"

void k_proc_Init();

uint32_t k_proc_CreateProcess(uint32_t start_address, void *image, uint32_t size);

struct k_proc_process_t *k_proc_GetProcess(uint32_t process_id);

struct k_proc_process_t *k_proc_GetCurrentProcess();

// struct k_proc_thread_t *k_proc_AllocThread();

// void k_proc_FreeThread(struct k_proc_thread_t *thread);

// struct k_proc_thread_t *k_proc_CreateThread(uintptr_t (*thread_fn)(void *data), void *data, uint32_t privilege_level);

// struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id);

// struct k_proc_thread_t *k_proc_GetCurrentThread();

// void k_proc_RunThreadCallback(struct k_proc_thread_t *thread);

// void k_proc_RunThread(struct k_proc_thread_t *thread);

// void k_proc_KillThread(uint32_t thread_id);

// void k_proc_SuspendThread(uint32_t thread_id);

// uint32_t k_proc_WaitThread(uint32_t thread_id);

void k_proc_RunScheduler();

uintptr_t k_proc_CleanupThread(void *data);

// void k_proc_Yield();

void k_proc_EnablePreemption();

void k_proc_DisablePreemption();

// extern void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

// uintptr_t func1(void *data);

// uintptr_t func2(void *data);

// uintptr_t func3(void *data);

// uintptr_t func4(void *data);

// uintptr_t func5(void *data);

// uintptr_t func6(void *data);

// uintptr_t func7(void *data);

#endif