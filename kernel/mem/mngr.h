#ifndef MNGR_H
#define MNGR_H

#include "defs.h"

void *k_mem_AllocVirtualRange(size_t size);

uint32_t k_mem_UsedVirtualRangeIndex(void *range);

void k_mem_FreeVirtualRange(void *mem);

void k_mem_SortFreePhysicalPages();

uintptr_t k_mem_AllocPhysicalPage(uint32_t flags);

uintptr_t k_mem_AllocPhysicalPages(uint32_t flags, uint32_t count);

void k_mem_FreePhysicalPages(uintptr_t page);

#endif