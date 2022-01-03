#ifndef K_THREAD_H
#define K_THREAD_H

#include "k_defs.h"

struct k_proc_thread_t *k_proc_CreateThread(uintptr_t (*thread_fn)(void *data), void *data);

struct k_proc_thread_t *k_proc_GetThread(uint32_t thread_id);

struct k_proc_thread_t *k_proc_GetCurrentThread();

void k_proc_DestroyThread(struct k_proc_thread_t *thread);

void k_proc_KillThread(uint32_t thread_id);

void k_proc_SuspendThread(uint32_t thread_id);

uint32_t k_proc_WaitThread(uint32_t thread_id);

void k_proc_Yield();

void k_proc_RunThreadCallback(struct k_proc_thread_t *thread);

void k_proc_RunThread(struct k_proc_thread_t *thread);

extern void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

#endif