#ifndef K_MEM_PHEAP_H
#define K_MEM_PHEAP_H

#include "k_mem_com.h"

void k_mem_SortFreePages();

uint32_t k_mem_AllocPage(uint32_t flags);

uint32_t k_mem_AllocPages(uint32_t page_count, uint32_t flags);

uint32_t k_mem_IsValidPage(uint32_t page_address);

void k_mem_FreePages(uint32_t page_address);

#endif