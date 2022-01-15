#ifndef K_THREAD_H
#define K_THREAD_H

#include "k_defs.h"

struct k_proc_thread_t *k_proc_CreateThread(uintptr_t (*thread_fn)(void *data), void *data);

void k_proc_DetachThread(struct k_proc_thread_t *thread);

struct k_proc_thread_t *k_proc_GetThread(uint32_t handle);

struct k_proc_thread_t *k_proc_GetCurrentThread();

void k_proc_DestroyThread(struct k_proc_thread_t *thread);

void k_proc_KillThread(struct k_proc_thread_t *thread);

void k_proc_SuspendThread(struct k_proc_thread_t *thread);

uint32_t k_proc_WaitThread(struct k_proc_thread_t *thread, uintptr_t *value);

// void k_proc_QueueThread(struct k_proc_thread_queue_t *queue, struct k_proc_thread_t *thread);

// void k_proc_UnqueueThread(struct k_proc_thread_t *thread);

// void k_proc_SetThreadReady(uint32_t thread_id);

void k_proc_QueueReadyThread(struct k_proc_thread_t *thread);

void k_proc_QueueDetachedThread(struct k_proc_thread_t *thread);

void k_proc_Yield();

void k_proc_RunThreadCallback(struct k_proc_thread_t *thread);

void k_proc_RunThread(struct k_proc_thread_t *thread);

extern void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

#endif