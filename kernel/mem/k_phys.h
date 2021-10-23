#ifndef K_PHYS_H
#define K_PHYS_H 

#include "k_defs.h"

void k_mem_SortFreePages();

uintptr_t k_mem_AllocPage(uint32_t flags);

uintptr_t k_mem_AllocPages(size_t page_count, uint32_t flags);

uintptr_t k_mem_IsValidPage(uintptr_t page_address);

void k_mem_FreePages(uintptr_t page_address);

#endif