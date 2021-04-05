#ifndef K_MEM_H
#define K_MEM_H

#include <stdint.h>

// struct k_mem_alloc_t
// {
//     struct k_mem_alloc_t *next;
//     struct k_mem_alloc_t *prev;
//     uint32_t size;
// };

struct k_mem_block_t
{
    uint32_t base;
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


enum K_MEM_SEG_FLAGS
{
    /* accessed */
    K_MEM_SEG_FLAG_A = 1 << 8,
    /* code segment when set, data segment otherwise */
    K_MEM_SEG_FLAG_C = 1 << 11
};

enum K_MEM_DSEG_FLAGS
{
    /* read/write segment when set, read-only otherwise */
    K_MEM_DSEG_FLAG_RW = 1 << 9,
    /* expand down segment */
    K_MEM_DSEG_FLAG_E = 1 << 10,
};

enum K_MEM_CSEG_FLAGS
{
    /* execute/read segment when set, execute-only otherwise */
    K_MEM_CSEG_FLAG_ER = 1 << 9,
    /* conforming segment (not to be confused with K_MEM_SEG_FLAG_C, 
        which tells whether the segment is a code or data segment) */
    K_MEM_CSEG_FLAG_C = 1 << 10
};

enum K_MEM_SYS_SEG_TYPES
{
    K_MEM_SYS_SEG_TYPE_TSS16_AVAL = 1 << 8,
    K_MEM_SYS_SEG_TYPE_LDT = 2 << 8,
    K_MEM_SYS_SEG_TYPE_TSS16_BUSY = 3 << 8,
    K_MEM_SYS_SEG_TYPE_CG16 = 4 << 8,
    K_MEM_SYS_SEG_TYPE_TG = 5 << 8,
    K_MEM_SYS_SEG_TYPE_IG16 = 6 << 8,
    K_MEM_SYS_SEG_TYPE_TG16 = 7 << 8,
    K_MEM_SYS_SEG_TYPE_TSS32_AVAL = 9 << 8,
    K_MEM_SYS_SEG_TYPE_TSS32_BUSY = 11 << 8,
    K_MEM_SYS_SEG_TYPE_CG32 = 12 << 8,
    K_MEM_SYS_SEG_TYPE_IG32 = 14 << 8,
    K_MEM_SYS_SEG_TYPE_TG32 = 15 << 8
};
struct k_mem_seg_desc_t
{
    uint32_t dw0;
    uint32_t dw1;
};

enum K_MEM_PAGE_FLAGS
{
    K_MEM_PAGE_FLAG_USED = 1,
    K_MEM_PAGE_FLAG_HEAD = 1 << 1,
    K_MEM_PAGE_FLAG_LAST
};

#define K_MEM_USED_PAGE_BITS 2

#define K_MEM_INVALID_PAGE 0x00000000
#define K_MEM_BIG_PAGE_ADDR_MASK 0xffc00000
#define K_MEM_SMALL_PAGE_ADDR_MASK 0xfffff000
#define K_MEM_SMALL_PAGE_USED_BYTE_MASK 0x0003ffff
#define K_MEM_SMALL_PAGE_USED_BYTE_SHIFT 14
#define K_MEM_SMALL_PAGE_USED_BYTE_INDEX(entry) (((entry) >> K_MEM_SMALL_PAGE_USED_BYTE_SHIFT) & K_MEM_SMALL_PAGE_USED_BYTE_MASK)
#define K_MEM_SMALL_PAGE_USED_BIT_MASK 0x00000006
#define K_MEM_SMALL_PAGE_USED_BIT_SHIFT 11
#define K_MEM_SMALL_PAGE_USED_BIT_INDEX(entry) (((entry) >> K_MEM_SMALL_PAGE_USED_BIT_SHIFT) & K_MEM_SMALL_PAGE_USED_BIT_MASK)
#define K_MEM_PENTRY0_SHIFT 22
#define K_MEM_PENTRY1_SHIFT 12
#define K_MEM_PENTRY_INDEX_MASK 0x000003ff
#define K_MEM_PENTRY_ADDR_MASK 0xfffff000
#define K_MEM_PENTRY1_ADDRESS(entry) ((struct k_mem_pentry_t *)((entry) & K_MEM_PENTRY_ADDR_MASK))
#define K_MEM_PENTRY0_INDEX(address) ((address >> K_MEM_PENTRY0_SHIFT) & K_MEM_PENTRY_INDEX_MASK)
#define K_MEM_PENTRY1_INDEX(address) ((address >> K_MEM_PENTRY1_SHIFT) & K_MEM_PENTRY_INDEX_MASK)

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

enum K_MEM_PAGING_STATUS
{
    K_MEM_PAGING_STATUS_OK = 0,
    K_MEM_PAGING_STATUS_NO_PTABLE = 1,
    K_MEM_PAGING_STATUS_PAGED = 2,
    K_MEM_PAGING_STATUS_NOT_PAGED = 3,
};

struct k_mem_pentry_t
{
    uint32_t entry;
};


void k_mem_init();

uint32_t k_mem_alloc_page();

uint32_t k_mem_alloc_pages(uint32_t count);

void k_mem_free_page(uint32_t page);

extern void k_mem_enable_paging();

extern void k_mem_disable_paging();

uint32_t k_mem_map_address(struct k_mem_pentry_t *page_dir, uint32_t page, uint32_t address, uint32_t flags);

uint32_t k_mem_unmap_address(struct k_mem_pentry_t *page_dir, uint32_t address);


#endif