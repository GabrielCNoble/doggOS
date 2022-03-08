#ifndef PMAP_H
#define PMAP_H

#include "defs.h"

// uint32_t k_mem_MapLinearAddressOnPageMap(uintptr_t page_map, uintptr_t linear_address, uintptr_t physical_address, uint32_t flags);

struct k_mem_pentry_t *k_mem_MapPageMapTable(uintptr_t page_map);

uint32_t k_mem_UnmapPageMapTable(struct k_mem_pentry_t *page_map_table);

uint32_t k_mem_MapLinearAddress(uintptr_t linear_address, uintptr_t physical_address, uint32_t flags);

uint32_t k_mem_UnmapLinearAddress(uintptr_t linear_address);

uintptr_t k_mem_LinearAddressPhysicalPage(uintptr_t linear_address);

uint32_t k_mem_IsLinearAddressMapped(uintptr_t linear_address);


#endif