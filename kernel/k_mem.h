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

enum K_MEM_SEG_DESC_FLAGS
{
    K_MEM_SEG_DESC_FLAG_CODE_DATA = 1 << 12,
    K_MEM_SEG_DESC_FLAG_PRESENT = 1 << 15,
    K_MEM_SEG_DESC_FLAG_64_BIT = 1 << 21,
    K_MEM_SEG_DESC_FLAG_DEFAULT_OP_SIZE = 1 << 22,
    K_MEM_SEG_DESC_FLAG_GRANULARITY = 1 << 23,
};

struct k_mem_seg_desc_t
{
    uint32_t dw0;
    uint32_t dw1;
};


#define K_MEM_BIG_PAGE_PHYS_ADDR_MASK 0xffc00000

enum K_MEM_PENTRY_FLAGS
{
    K_MEM_PENTRY_FLAG_PRESENT = 1,
    K_MEM_PENTRY_FLAG_READ_WRITE = 1 << 1,
    K_MEM_PEMTRY_FLAG_USER_SUPERVISOR = 1 << 2,
    K_MEM_PENTRY_FLAG_PAGE_WRITE_THROUGH = 1 << 3,
    K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE = 1 << 4,
    K_MEM_PENTRY_FLAG_ACCESSED = 1 << 5,
    K_MEM_PENTRY_FLAG_DIRTY = 1 << 6,
    K_MEM_PENTRY_FLAG_BIG_PAGE = 1 << 7,
    K_MEM_PENTRY_FLAG_GLOBAL = 1 << 8,
    K_MEM_PENTRY_FLAG_USED = 1 << 9,


    K_MEM_PENTRY_FLAG_SMALL_PAGE_PAT = 1 << 7,
    K_MEM_PENTRY_FLAG_BIG_PAGE_PAT = 1 << 12,
};

struct k_mem_pentry_t
{
    uint32_t entry;
};


void k_mem_init();

void *k_mem_alloc(uint32_t size);

void *k_mem_realloc(void *memory, uint32_t new_size);

void k_mem_free(void *mem);

extern void k_mem_enable_paging();

extern void k_mem_disable_paging();

uint32_t k_mem_page_mem(uint32_t physical_address, uint32_t linear_address, uint32_t big_page);


#endif