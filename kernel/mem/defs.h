#ifndef MEM_DEFS_H
#define MEM_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

#include "../rt/atm.h"

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

enum K_MEM_PAGE_FLAGS
{
    /* page is in... well... use */
    K_MEM_PAGE_FLAG_USED = 1,
    /* page is the first page in an allocation of contiguous pages */
    K_MEM_PAGE_FLAG_HEAD = 1 << 1,
    /* pinned page, which shouldn't be swapped */
    K_MEM_PAGE_FLAG_PINNED = 1 << 2,
    /* page being used in a pipe buffer, which means it's owned by several 
    processes and requires special care when being freed */
    K_MEM_PAGE_FLAG_PIPE = 1 << 3,

    K_MEM_PAGE_FLAG_SHARED = 1 << 4,

    K_MEM_PAGE_FLAG_LAST,
};

/* used physical page. This header has two uses. When it references an allocated page, it keeps both
the id of the owner process, and also some flags. When it's not in use, it keeps the index into the
free list where the page is located. This is useful for quick defragmentation.  */
struct k_mem_upage_t
{
    uint32_t index : 24;
    uint32_t flags : 8;   
    // uint32_t index;
    // uint32_t flags;
}; 

struct k_mem_prange_t
{
    uintptr_t base_address;
    uint32_t *free_pages;
    struct k_mem_upage_t *used_pages;
    uint32_t free_count;
    uint32_t page_count;
};

struct k_mem_prlist_t
{
    uint32_t range_count;
    struct k_mem_prange_t *ranges;
};

struct k_mem_vrange_t
{
    uint32_t start;
    uint32_t size;
};

/* list of virtual ranges. This list is used for both free and used ranges.
Free ranges live from offest 0 to offset free_count - 1; used ranges live
from offset free_count to range_count - 1 */
struct k_mem_vrlist_t
{
    uint32_t free_count;
    uint32_t range_count;
    struct k_mem_vrange_t *ranges;
};

// #define K_MEM_USED_PAGE_BITS 4

#define K_MEM_4KB_ADDRESS_SHIFT 12u
#define K_MEM_4KB_ADDRESS_MASK 0xfffff000u
#define K_MEM_4KB_INDEX_MASK 0x0003ffffu
#define K_MEM_4KB_ADDRESS_OFFSET 0x00001000u

#define K_MEM_4MB_ADDRESS_SHIFT 22u
#define K_MEM_4MB_ADDRESS_MASK 0xffc00000u
#define K_MEM_4MB_INDEX_MASK 0x000003ffu
#define K_MEM_4MB_ADDRESS_OFFSET 0x00400000u 

#define K_MEM_NULL_PAGE ((uintptr_t)0x00000000u)
#define K_MEM_BIG_PAGE_ADDR_MASK K_MEM_4MB_ADDRESS_MASK
#define K_MEM_SMALL_PAGE_ADDR_MASK K_MEM_4KB_ADDRESS_MASK

#define K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(address) (((address) >> K_MEM_4KB_ADDRESS_SHIFT) & K_MEM_4KB_INDEX_MASK)
#define K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX 0x00ffffffu

#define K_MEM_PDIR_SHIFT 22u
#define K_MEM_PDIR_ADDRESS_OFFSET K_MEM_4MB_ADDRESS_OFFSET
#define K_MEM_PTABLE_SHIFT 12u
#define K_MEM_PTABLE_ADDRESS_OFFSET K_MEM_4KB_ADDRESS_OFFSET
#define K_MEM_PENTRY_INDEX_MASK 0x000003ffu
#define K_MEM_PENTRY_ADDR_MASK 0xfffff000u

#define K_MEM_PTABLE_INDEX(address) ((address >> K_MEM_PTABLE_SHIFT) & K_MEM_PENTRY_INDEX_MASK)
#define K_MEM_PTABLE_ADDRESS(entry) ((struct k_mem_pentry_t *)((entry) & K_MEM_PENTRY_ADDR_MASK))
#define K_MEM_PDIR_INDEX(address) ((address >> K_MEM_PDIR_SHIFT) & K_MEM_PENTRY_INDEX_MASK)

enum K_MEM_PENTRY_FLAGS
{
    K_MEM_PENTRY_FLAG_PRESENT = 1,
    /* marks memory mapped as read/write. Otherwise, it's read only */
    K_MEM_PENTRY_FLAG_READ_WRITE = 1 << 1,
    K_MEM_PENTRY_FLAG_USER_MODE_ACCESS = 1 << 2,
    K_MEM_PENTRY_FLAG_PAGE_WRITE_THROUGH = 1 << 3,
    /* marks memory as not cacheable */
    K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE = 1 << 4,
    /* marks memory as accessed by the cpu. Set by the cpu, cleared by software */
    K_MEM_PENTRY_FLAG_ACCESSED = 1 << 5,
    /* marks memory as modified by the cpu. Set by the cpu, cleared by software */
    K_MEM_PENTRY_FLAG_DIRTY = 1 << 6,
    /* marks paging entry as mapping a big page (4MB) */
    K_MEM_PENTRY_FLAG_BIG_PAGE = 1 << 7,
    /* marks paging entry as global, which means it has priority in the tlb, and
    all non global entries will be flushed before it */
    K_MEM_PENTRY_FLAG_GLOBAL = 1 << 8,
    /* marks a entry as used by the paging system. This is not a flag the cpu cares about */
    K_MEM_PENTRY_FLAG_USED = 1 << 9,


    K_MEM_PENTRY_FLAG_SMALL_PAGE_PAT = 1 << 7,
    K_MEM_PENTRY_FLAG_BIG_PAGE_PAT = 1 << 12,
};

enum K_MEM_PAGING_STATUS
{
    K_MEM_PAGING_STATUS_OK = 0,
    K_MEM_PAGING_STATUS_NO_PTABLE = 1,
    K_MEM_PAGING_STATUS_ALREADY_USED = 2,
    K_MEM_PAGING_STATUS_NOT_PAGED = 3,
    K_MEM_PAGING_STATUS_NOT_ALLOWED = 4,
};

/* represents a page directory/page table entry */
struct k_mem_pentry_t
{
    uintptr_t entry;
};

struct k_mem_pmap_t
{
    struct k_mem_pentry_t *pdir;
    struct k_mem_pentry_t *ptable;
};

#define K_MEM_PMAP_PDIR_INDEX 0x3ff
#define K_MEM_PMAP_PTABLE_INDEX 0x3fe

/* 
    some magic pointers... 
*/

/* 
    This gives a pointer to the physical page that contains an active paging structure, whether that's
    the page dir. index 0x03ff gives the page that holds the page dir, 0x03fe gives the page that holds
    the page table that maps both the page dir and itself. Indices from 0x03fd to 0x0000 are used to map
    pages of other page tables, so their entries can be modified. Those indices are page dir entry indices.
    So, pdir entry 0 maps to index 0, pdir entry 1 maps to index 1, and so on.

    So, for the page dir, this will give 0xfffff000, for the page table it'll give 0xffffe000, and for
    the rest of indices, 0xffffe000 to 0xffc00000.
*/
#define K_MEM_PMAP_PENTRY_PAGE_POINTER(table_index) ((struct k_mem_pentry_t *)(0xffc00000 + ((table_index) << K_MEM_4KB_ADDRESS_SHIFT)))

/* this points to the physical page that contains the active pdir, and allows modifying its entries */
#define K_MEM_PMAP_PDIR_PAGE_POINTER K_MEM_PMAP_PENTRY_PAGE_POINTER(K_MEM_PMAP_PDIR_INDEX)
#define K_MEM_PMAP_PTABLE_PAGE_POINTER K_MEM_PMAP_PENTRY_PAGE_POINTER(K_MEM_PMAP_PTABLE_INDEX)




#endif