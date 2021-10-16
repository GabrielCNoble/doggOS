#ifndef K_MEM_H
#define K_MEM_H

#include "k_defs.h"
#include "k_alloc.h"
#include "k_phys.h"
#include "k_pmap.h"

void k_mem_Init(struct k_mem_range_t *ranges, uint32_t range_count);

#endif