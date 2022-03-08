#ifndef MEM_H
#define MEM_H

#include "defs.h"
#include "mngr.h"
#include "pmap.h"

void k_mem_MapKernelData(struct k_mem_pentry_t *pdir, uintptr_t address, uint32_t size);

void k_mem_Init(struct k_mem_range_t *ranges, uint32_t range_count);

#endif