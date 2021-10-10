#ifndef K_MEM_DEFS_H
#define K_MEM_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

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

// enum K_MEM_SEG_DESC_FLAGS
// {
//     K_MEM_SEG_DESC_FLAG_CODE_DATA = 1 << 12,
//     K_MEM_SEG_DESC_FLAG_PRESENT = 1 << 15,
//     K_MEM_SEG_DESC_FLAG_64_BIT = 1 << 21,
//     K_MEM_SEG_DESC_FLAG_DEFAULT_OP_SIZE = 1 << 22,
//     K_MEM_SEG_DESC_FLAG_GRANULARITY = 1 << 23,
// };


// enum K_MEM_SEG_FLAGS
// {
//     /* accessed */
//     K_MEM_SEG_FLAG_A = 1 << 8,
//     /* code segment when set, data segment otherwise */
//     K_MEM_SEG_FLAG_C = 1 << 11
// };

// enum K_MEM_DSEG_FLAGS
// {
//     /* read/write segment when set, read-only otherwise */
//     K_MEM_DSEG_FLAG_RW = 1 << 9,
//     /* expand down segment */
//     K_MEM_DSEG_FLAG_E = 1 << 10,
// };

// enum K_MEM_CSEG_FLAGS
// {
//     /* execute/read segment when set, execute-only otherwise */
//     K_MEM_CSEG_FLAG_ER = 1 << 9,
//     /* conforming segment (not to be confused with K_MEM_SEG_FLAG_C, 
//         which tells whether the segment is a code or data segment) */
//     K_MEM_CSEG_FLAG_C = 1 << 10
// };

// enum K_MEM_SYS_SEG_TYPES
// {
//     K_MEM_SYS_SEG_TYPE_TSS16_AVAL = 1 << 8,
//     K_MEM_SYS_SEG_TYPE_LDT = 2 << 8,
//     K_MEM_SYS_SEG_TYPE_TSS16_BUSY = 3 << 8,
//     K_MEM_SYS_SEG_TYPE_CG16 = 4 << 8,
//     K_MEM_SYS_SEG_TYPE_TG = 5 << 8,
//     K_MEM_SYS_SEG_TYPE_IG16 = 6 << 8,
//     K_MEM_SYS_SEG_TYPE_TG16 = 7 << 8,
//     K_MEM_SYS_SEG_TYPE_TSS32_AVAL = 9 << 8,
//     K_MEM_SYS_SEG_TYPE_TSS32_BUSY = 11 << 8,
//     K_MEM_SYS_SEG_TYPE_CG32 = 12 << 8,
//     K_MEM_SYS_SEG_TYPE_IG32 = 14 << 8,
//     K_MEM_SYS_SEG_TYPE_TG32 = 15 << 8
// };
// struct k_mem_seg_desc_t
// {
//     uint32_t dw0;
//     uint32_t dw1;
// };

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

    K_MEM_PAGE_FLAG_EXCLUSIVE = 1 << 4,

    K_MEM_PAGE_FLAG_LAST,
};

/* used by the physical page allocator for allocated pages. 'pid_index' have different meanings, depending
on whether the page is allocated or not. When the page is allocated, it contains the pid of the process
that owns the page. When it's not allocated, it contains the index into the available physical page where
the entry */
struct k_mem_uppentry_t
{
    uint32_t pid_index : 24;
    uint32_t flags : 8;
};

struct k_mem_pheap_t
{
    uint32_t first_used_page;
    uint32_t last_used_page;
    /* used pages list. The page address is used as an index into this list, which means each entry 
    refers to the same page. For entries of pages that are not in use, it refers to the entry in the 
    available pages list that specific page is located. For example, if the entry for the first page 
    contains an index of 4, that means the first page is located in the fifth entry of the available
    pages list. */
    struct k_mem_uppentry_t *used_pages;

    uint32_t *free_pages;
    /* number of free pages in the list */
    uint32_t free_pages_count;
    /* maximum number of pages in the list, ever */
    uint32_t free_pages_max_count;
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

#define K_MEM_INVALID_PAGE 0x00000000u
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


/* page directory index of where the pstate and all the ptables are mapped */
#define K_MEM_PSTATE_SELF_PDIR_INDEX 0x000003ffu
/* page directory index of the last usable page directory. Because the last four entries of a pstate 
page table are used for its four pages, and the remaining entries each represent a page table used by
a page dir, the entries 0x3ff, 0x3fe, 0x3fd and 0x3fc are not available to map page tables for page dir 
entries. This effectively makes the last 16MB of the 4GB address space not usable. */
#define K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX 0x000003fbu

/* page table index of the page that contains the pstate */
#define K_MEM_PSTATE_SELF_PTABLE_INDEX 0x000003fdu
/* page table index of the page that contains the pstate pdir */
#define K_MEM_PSTATE_PDIR_PTABLE_INDEX 0x000003feu
/* page table index of the page that contains the pstate ptable */
#define K_MEM_PSTATE_PTABLE_PTABLE_INDEX 0x000003ffu



#define K_MEM_PSTATE_BASE_MAP_ADDRESS(block_address) ((block_address) & K_MEM_4MB_ADDRESS_MASK)
#define K_MEM_PSTATE_MAP_ADDRESS(block_address) (K_MEM_PSTATE_BASE_MAP_ADDRESS(block_address) + K_MEM_PSTATE_SELF_PTABLE_INDEX * K_MEM_4KB_ADDRESS_OFFSET)
#define K_MEM_PSTATE_PDIR_MAP_ADDRESS(block_address) (K_MEM_PSTATE_BASE_MAP_ADDRESS(block_address) + K_MEM_PSTATE_PDIR_PTABLE_INDEX * K_MEM_4KB_ADDRESS_OFFSET)
#define K_MEM_PSTATE_PTABLE_MAP_ADDRESS(block_address) (K_MEM_PSTATE_BASE_MAP_ADDRESS(block_address) + K_MEM_PSTATE_PTABLE_PTABLE_INDEX * K_MEM_4KB_ADDRESS_OFFSET)
#define K_MEM_PSTATE_PDIR_PTABLES_MAP_ADDRESS(block_address) K_MEM_PSTATE_BASE_MAP_ADDRESS(block_address)


/* "magic" pointers, only valid for the active pstate. Those are fixed values because they need to always 
map to the same page dir and page table entries. The upper 10 bits of it are 0xffc00000, which maps to the 
last page dir entry. The next 10 bits are 0x03fd0000, 0x03fe0000 or 0x03ff0000, which map to the three last 
page table entries of the active pstate. The rest of the values map to the rest of the page table entries. */
#define K_MEM_ACTIVE_PSTATE_BLOCK_ADDRESS (K_MEM_4MB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PDIR_INDEX)
#define K_MEM_ACTIVE_PSTATE_INIT_PSTATE_BLOCK_ADDRESS (K_MEM_4MB_ADDRESS_OFFSET * (K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX + 1))
#define K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER ((struct k_mem_pentry_t *)K_MEM_PSTATE_PDIR_MAP_ADDRESS(K_MEM_ACTIVE_PSTATE_BLOCK_ADDRESS))
#define K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER ((struct k_mem_pentry_t *)K_MEM_PSTATE_PTABLE_MAP_ADDRESS(K_MEM_ACTIVE_PSTATE_BLOCK_ADDRESS))
#define K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER ((struct k_mem_pentry_page_t *)K_MEM_PSTATE_PDIR_PTABLES_MAP_ADDRESS(K_MEM_ACTIVE_PSTATE_BLOCK_ADDRESS))

#define K_MEM_ACTIVE_MAPPED_PSTATE ((struct k_mem_pstate_t){.page_dir = K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER, \
                                                            .page_table = K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER, \
                                                            .page_dir_ptables = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER})

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
    uint32_t entry;
};

/* represents a page directory/page table. Used to facilitate modifications to page
tables */
struct k_mem_pentry_page_t
{
    struct k_mem_pentry_t entries[1024];
};


/* this struct contains the address of the three physical pages used by a pstate */ 
// struct k_mem_pstate_t
// {
//     /* page that contains this struct */
//     uint32_t self_page;
//     /* page that contains the page directory used by this pstate */
//     uint32_t pdir_page;
//     /* page that contains the page table used by this pstate. This is a "special" page table, because not only 
//     it maps the four physical pages used by the pstate to the four fixed linear addresses 0xffffc000, 0xffffd000, 
//     0xffffe000 and 0xfffff000 (K_MEM_ACTIVE_PSTATE_BLOCK_TABLE, K_MEM_ACTIVE_PSTATE, K_MEM_ACTIVE_PSTATE_PDIR and 
//     K_MEM_ACTIVE_PSTATE_PTABLE, respectivelly), but is also used to map the physical pages that the page directory 
//     uses for its page tables, so they're can be modified. This makes them accessible from addresses 0xffc00000 to 
//     0xffffb000, where 0xffc00000 is the page table of the first page directory entry and 0xffffc000 is the page table 
//     of the last usable page directory. Since the last four entries of this page table are being used for the pstate, 
//     and each remaining entry represent the page table of a page directory entry, it means that the last 16MB of linear 
//     addresses aren't available for use by anything else. That sounds like a lot, but it amounts to 1/341 of the 4GB 
//     address space, so it's not terrible. */
//     uint32_t ptable_page;

//     /* page that contains the allocated block table */
//     uint32_t block_table_page;
// };

struct k_mem_pstate_h
{
    uint32_t self_page;
    uint32_t pdir_page;
    uint32_t ptable_page;
};
struct k_mem_pstate_t
{
    // struct k_mem_heapm_t heapm;
    /* linear address of the page dir. This is valid only when this pstate is mapped. */
    struct k_mem_pentry_t *page_dir;
    /* linear address of the page tables used by the page directory */
    struct k_mem_pentry_page_t *page_dir_ptables;
    /* linear address of the page that contains the page table */
    struct k_mem_pentry_t *page_table;
};

// struct k_mem_aspace_t
// {
//     struct k_mem_pstate_t pstate;
//     struct k_mem_heapm_t heapm;
// };

struct k_mem_bchunk_t
{
    struct k_mem_bchunk_t *next;
    struct k_mem_bchunk_t *prev;
    uint32_t size;
};

struct k_mem_schunk_t
{
    struct k_mem_schunk_t *next;
    struct k_mem_schunk_t *prev;
};

struct k_mem_scpage_t
{
    struct k_mem_scpage_t *next;
    uint32_t main_bucket: 4;
    uint32_t free_count: 12;

    struct k_mem_schunk_t chunks[511];
};

struct k_mem_sbucket_t
{
    struct k_mem_schunk_t *first_chunk;
    struct k_mem_schunk_t *last_chunk;
};

#define K_MEM_VHEAP_BUCKET_COUNT 9

struct k_mem_vheap_t
{
    struct k_mem_sbucket_t buckets[K_MEM_VHEAP_BUCKET_COUNT];
    void *source_block;
    // struct k_mem_bchunk_t *first_free_chunk;
    // struct k_mem_bchunk_t *last_free_chunk;
};

struct k_mem_state_t
{
    struct k_mem_pstate_h pstate;
    struct k_mem_vheap_t vheap;
    struct k_mem_pheap_t pheap;
};

#endif