#ifndef K_PROC_H
#define K_PROC_H

#include "defs.h"

void k_proc_Init();

struct k_proc_process_t *k_proc_CreateProcess(void *image, const char *path, const char **args);

struct k_proc_process_t *k_proc_LaunchProcess(const char *path, const char **args);

uint32_t k_proc_WaitProcess(struct k_proc_process_t *process, uintptr_t *return_value);

struct k_proc_process_t *k_proc_GetProcess(uint32_t process_id);

struct k_proc_process_t *k_proc_GetCurrentProcess();

struct k_proc_process_t *k_proc_GetFocusedProcess();

void k_proc_RunScheduler();

uintptr_t k_proc_CleanupThread(void *data);

void k_proc_EnablePreemption();

void k_proc_DisablePreemption();

#endif
