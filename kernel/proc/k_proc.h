#ifndef K_PROC_H
#define K_PROC_H

#include "k_defs.h"

void k_proc_Init();

uint32_t k_proc_CreateProcess(uint32_t start_address, void *image, uint32_t size);

struct k_proc_process_t *k_proc_GetProcess(uint32_t process_id);

struct k_proc_process_t *k_proc_GetCurrentProcess();

uint32_t k_proc_CreateThread(void (*thread_fn)(), uint32_t privilege_level);

struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id);

struct k_proc_thread_t *k_proc_GetCurrentThread();

void k_proc_KillThread(uint32_t thread_id);

void k_proc_SuspendThread(uint32_t thread_id);

void k_proc_RunScheduler();

void k_proc_Yield();

extern void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

void func1();

void func2();

void func3();

void func4();

void func5();

void func6();

void func7();

#endif