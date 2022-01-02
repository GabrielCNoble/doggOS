#ifndef DG_THREAD_H
#define DG_THREAD_H

#include "dg_defs.h"


struct dg_thread_t *dg_CreateThread(uint32_t (*thread_func)(void *data));

void dg_StopThread(struct dg_thread_t *thread);

uint32_t dg_WaitThread(struct dg_thread_t *thread);


#endif