#ifndef PMAP_H
#define PMAP_H

#include "defs.h"


uint32_t k_mem_MapLinearAddress(uintptr_t linear_address, uintptr_t physical_address, uint32_t flags);

uint32_t k_mem_UnmapLinearAddress(uintptr_t linear_address);

uint32_t k_mem_IsLinearAddressMapped(uintptr_t linear_address);


#endif