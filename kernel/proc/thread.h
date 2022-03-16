#ifndef K_THREAD_H
#define K_THREAD_H

#include "defs.h"
#include "../io.h"

struct k_proc_thread_t *k_proc_CreateKernelThread(k_proc_thread_func_t entry_point, void *user_data);

struct k_proc_thread_t *k_proc_CreateThread(struct k_proc_thread_init_t *init);

// struct k_proc_thread_t *k_proc_InitKernelThread()

void k_proc_DetachThread(struct k_proc_thread_t *thread);

struct k_proc_thread_t *k_proc_GetThread(uint32_t handle);

struct k_proc_thread_t *k_proc_GetCurrentThread();

void k_proc_DestroyThread(struct k_proc_thread_t *thread);

void k_proc_SuspendThread(struct k_proc_thread_t *thread);

uint32_t k_proc_WaitThread(struct k_proc_thread_t *thread, uintptr_t *value);

uint32_t k_proc_WaitStream(struct k_io_stream_t *stream);

void k_proc_YieldThread();

void k_proc_StartUserThread(k_proc_thread_func_t entry_point, void *user_data);

void k_proc_StartKernelThread(k_proc_thread_func_t entry_point, void *user_data);

void k_proc_TerminateThread(uintptr_t return_value);

void k_proc_RunThread(struct k_proc_thread_t *thread);

void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

void k_proc_QueueReadyThread(struct k_proc_thread_t *thread);

void k_proc_QueueWaitingThread(struct k_proc_thread_t *thread);

void k_proc_QueueIOBlockedThread(struct k_proc_thread_t *thread);

void k_proc_QueueDetachedThread(struct k_proc_thread_t *thread);

#endif