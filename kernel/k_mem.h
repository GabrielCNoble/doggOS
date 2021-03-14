#ifndef K_MEM_H
#define K_MEM_H

#include <stdint.h>

struct k_mem_alloc_t
{
    struct k_mem_alloc_t *next;
    struct k_mem_alloc_t *prev;
    uint32_t size;
};

enum K_MEM_RANGE_TYPES
{
    K_MEM_RANGE_TYPE_FREE = 1,
    K_MEM_RANGE_TYPE_RESERVED = 2,
    K_MEM_RANGE_TYPE_ACPI_RECLAIM = 3,
    K_MEM_RANGE_TYPE_ACPI_NVS = 4
};

struct k_mem_range_t
{
    uint64_t base;
    uint64_t size;
    uint64_t type;
};


void k_mem_init();

void *k_mem_alloc(uint32_t size);

void k_mem_free(void *mem);

#endif