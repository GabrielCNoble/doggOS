#ifndef K_MEM_H
#define K_MEM_H

#include "k_mem_com.h"
#include "k_mem_vheap.h"
#include "k_mem_pheap.h"
#include "k_mem_pstate.h"

void k_mem_Init(struct k_mem_range_t *ranges, uint32_t range_count);

#endif