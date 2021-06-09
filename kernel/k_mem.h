#ifndef K_MEM_H
#define K_MEM_H

#include <stdint.h>

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



/* 0 - 255 bytes */
#define K_MEM_MINUSCULE_BUCKET_SIZE 0x000000ff
/* 256 - 4095 bytes */
#define K_MEM_TINY_BUCKET_SIZE 0x00000fff
/* 4096 - 1048575 bytes */
#define K_MEM_SMALL_BUCKET_SIZE 0x000fffff
/* 1048576 - 16777215 bytes */
#define K_MEM_MEDIUM_BUCKET_SIZE 0x00ffffff
/* 16777216 - 134217727 bytes */
#define K_MEM_LARGE_BUCKET_SIZE 0x07ffffff
/* 134217727 bytes onwards */
#define K_MEM_HUGE_BUCKET_SIZE 0xffffffff

enum K_MEM_BUCKETS
{
    K_MEM_BUCKET_MINUSCULE = 0,
    K_MEM_BUCKET_TINY,
    K_MEM_BUCKET_SMALL,
    K_MEM_BUCKET_MEDIUM,
    K_MEM_BUCKET_LARGE,
    K_MEM_BUCKET_HUGE,
    K_MEM_BUCKET_LAST
};

// struct k_mem_bucket_entry_t
// {
//     struct k_mem_block_t *block;
//     uint32_t block_size;
// };

// struct k_mem_block_t
// {
//     struct k_mem_block_t *left;
//     struct k_mem_block_t *right;
//     struct k_mem_bucket_entry_t *bucket_entry;
// };

// enum K_MEM_BUCKET_FLAGS
// {
//     K_MEM_BUCKET_FLAG_NEEDS_REFRESH = 1,
// };

// struct k_mem_bucket_t
// {
//     struct k_mem_bucket_t *prev;
//     uint16_t header_cursor;
//     uint16_t flags;
//     struct k_mem_bucket_entry_t entries[1023];
// };

// struct k_mem_heap_t
// {
//     struct k_mem_bucket_t *buckets[K_MEM_BUCKET_LAST];
// };

/* this is just a temporary implementation of the heap manager. This implementation
has O(1) deallocation in all cases, O(1) allocation in the best case, O(n) in the worst,
and O(n^2) in defragmentation. The final implementation will use a rb tree, sorted by start
address, to allow deallocation in O(log n) and defragmentation on insertion. Allocation will
use buckets formed by linked lists of references to blocks, to allow O(m) allocation in the
worst case, where m <= n. */

enum K_MEM_BLOCK_FLAGS
{
    K_MEM_BLOCK_FLAG_MAPPED = 1,
};

#define K_MEM_MIN_ALLOC_SIZE sizeof(struct k_mem_block_t)
#define K_MEM_BLOCK_SIZE_MASK 0xfffffff8
struct k_mem_block_t
{
    struct k_mem_block_t *next;
    struct k_mem_block_t *prev;
    uint32_t size;
    // uint32_t size : 29;
    // uint32_t flags : 3;
    // uint32_t size;
};

struct k_mem_heapm_t
{
    struct k_mem_block_t *blocks;
    struct k_mem_block_t *last_block;
    uint32_t block_count;
};


enum K_MEM_PAGE_FLAGS
{
    K_MEM_PAGE_FLAG_USED = 1,
    K_MEM_PAGE_FLAG_HEAD = 1 << 1,
    K_MEM_PAGE_FLAG_LAST
};

#define K_MEM_USED_PAGE_BITS 2

#define K_MEM_4KB_ADDRESS_OFFSET 0x00001000u
#define K_MEM_4MB_ADDRESS_OFFSET 0x00400000u

#define K_MEM_INVALID_PAGE 0x00000000u
#define K_MEM_BIG_PAGE_ADDR_MASK 0xffc00000u
#define K_MEM_SMALL_PAGE_ADDR_MASK 0xfffff000u
#define K_MEM_SMALL_PAGE_USED_BYTE_MASK 0x0003ffffu
#define K_MEM_SMALL_PAGE_USED_BYTE_SHIFT 14
#define K_MEM_SMALL_PAGE_USED_BYTE_INDEX(entry) (((entry) >> K_MEM_SMALL_PAGE_USED_BYTE_SHIFT) & K_MEM_SMALL_PAGE_USED_BYTE_MASK)
#define K_MEM_SMALL_PAGE_USED_BIT_MASK 0x00000006u
#define K_MEM_SMALL_PAGE_USED_BIT_SHIFT 11
#define K_MEM_SMALL_PAGE_USED_BIT_INDEX(entry) (((entry) >> K_MEM_SMALL_PAGE_USED_BIT_SHIFT) & K_MEM_SMALL_PAGE_USED_BIT_MASK)
#define K_MEM_PDIR_SHIFT 22
#define K_MEM_PDIR_ADDRESS_OFFSET K_MEM_4MB_ADDRESS_OFFSET
#define K_MEM_PTABLE_SHIFT 12
#define K_MEM_PTABLE_ADDRESS_OFFSET K_MEM_4KB_ADDRESS_OFFSET
#define K_MEM_PENTRY_INDEX_MASK 0x000003ffu
#define K_MEM_PENTRY_ADDR_MASK 0xfffff000u

#define K_MEM_PTABLE_INDEX(address) ((address >> K_MEM_PTABLE_SHIFT) & K_MEM_PENTRY_INDEX_MASK)
#define K_MEM_PTABLE_ADDRESS(entry) ((struct k_mem_pentry_t *)((entry) & K_MEM_PENTRY_ADDR_MASK))
#define K_MEM_PDIR_INDEX(address) ((address >> K_MEM_PDIR_SHIFT) & K_MEM_PENTRY_INDEX_MASK)


/* page directory index of where the pstate and all the ptables are mapped */
#define K_MEM_PSTATE_SELF_PDIR_INDEX 0x000003ffu
/* page directory index of the last usable page directory. Because the last three entries of a pstate 
page table are used for its three pages, and the remaining entries each represent a page table used by
a page dir, the entries 0x3ff, 0x3fe and 0x3fd are not available to map page tables for page dir entries.
This effectively makes the last 12MB of the 4GB address space not usable. */
#define K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX 0x000003fcu
/* page table index of the page that contains the pstate */
#define K_MEM_PSTATE_SELF_PTABLE_INDEX 0x000003fdu
/* page table index of the page that contains the pstate pdir */
#define K_MEM_PSTATE_PDIR_PTABLE_INDEX 0x000003feu
/* page table index of the page that contains the pstate ptable */
#define K_MEM_PSTATE_PTABLE_PTABLE_INDEX 0x000003ffu

#define K_MEM_ACTIVE_PSTATE ((struct k_mem_pstate_p *)0xffffd000)
#define K_MEM_ACTIVE_PSTATE_HEAP_MANAGER ((struct k_mem_heapm_t *)(K_MEM_ACTIVE_PSTATE + offsetof(struct k_mem_pstate_p, heapm))) 
#define K_MEM_ACTIVE_PSTATE_PDIR ((struct k_mem_pentry_t *)0xffffe000)
#define K_MEM_ACTIVE_PSTATE_PTABLE ((struct k_mem_pentry_t *)0xfffff000)
#define K_MEM_ACTIVE_PSTATE_PDIR_PTABLES ((struct k_mem_pentry_page_t *)0xffc00000)

enum K_MEM_PENTRY_FLAGS
{
    K_MEM_PENTRY_FLAG_PRESENT = 1,
    K_MEM_PENTRY_FLAG_READ_WRITE = 1 << 1,
    K_MEM_PENTRY_FLAG_USER_SUPERVISOR = 1 << 2,
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
    K_MEM_PAGING_STATUS_ALREADY_USED = 2,
    K_MEM_PAGING_STATUS_NOT_PAGED = 3,
    K_MEM_PAGING_STATUS_NOT_ALLOWED = 4,
};


struct k_mem_pentry_t
{
    uint32_t entry;
};

struct k_mem_pentry_page_t
{
    struct k_mem_pentry_t entries[1024];
};

/* this struct contains the address of the three physical pages used by a pstate */ 
struct k_mem_pstate_t
{
    /* page that contains this struct */
    uint32_t self_page;
    /* page that contains the page directory used by this pstate */
    uint32_t pdir_page;
    /* page that contains the page table used by this pstate. This is a "special" page table, because not only 
    it maps the three physical pages used by the pstate to the three fixed linear addresses 0xffffd000, 0xffffe000 
    and 0xfffff000 (K_MEM_ACTIVE_PSTATE, K_MEM_ACTIVE_PSTATE_PDIR and K_MEM_ACTIVE_PSTATE_PTABLE, respectivelly), 
    but is also used to map the physical pages that the page directory uses for its page tables, so they're can be 
    modified. This makes them accessible from addresses 0xffc00000 to 0xffffc000, where 0xffc00000 is the page table 
    of the first page directory entry and 0xffffc000 is the page table of the last usable page directory. Since the 
    last three entries of this page table are being used for the pstate, and each remaining entry represent the page 
    table of a page directory entry, it means that the last 12MB of linear addresses aren't available for use by 
    anything else. That sounds like a lot, but it amounts to 1/341 of the 4GB address space, so it's not terrible. */
    uint32_t ptable_page;
};
struct k_mem_pstate_p
{
    struct k_mem_heapm_t heapm;
    /* linear address of the page dir. This is valid only when this pstate is mapped. */
    struct k_mem_pentry_t *page_dir;
    /* linear address of the page tables used by the page directory */
    struct k_mem_pentry_page_t *page_dir_ptables;
    /* linear address of the page that contains the page table */
    struct k_mem_pentry_t *page_table;
};

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

void k_mem_init();

uint32_t k_mem_alloc_page();

uint32_t k_mem_alloc_pages(uint32_t count);

void k_mem_free_pages(uint32_t page_address);

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

struct k_mem_pstate_t k_mem_create_pstate();

struct k_mem_pstate_p *k_mem_map_pstate(struct k_mem_pstate_t *pstate);

struct k_mem_pstate_p *k_mem_map_pstate_to_address(struct k_mem_pstate_t *pstate, uint32_t map_address);

void k_mem_unmap_pstate(struct k_mem_pstate_p *pstate);

void k_mem_destroy_pstate(struct k_mem_pstate_t *pstate);

extern void k_mem_load_page_dir(uint32_t pdir_page);

void k_mem_load_pstate(struct k_mem_pstate_t *pstate);

extern struct k_mem_pstate_t *k_mem_get_pstate();

extern uint32_t k_mem_paging_enabled();

extern void k_mem_enable_paging();

extern void k_mem_disable_paging();

uint32_t k_mem_map_temp(uint32_t phys_address);

uint32_t k_mem_map_address(struct k_mem_pstate_p *pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags);

uint32_t k_mem_set_entry(struct k_mem_pentry_t *dir_entry, struct k_mem_pentry_t *page_entry, uint32_t phys_address, uint32_t flags);

uint32_t k_mem_address_mapped(struct k_mem_pstate_p *pstate, uint32_t address);

uint32_t k_mem_unmap_address(struct k_mem_pstate_p *pstate, uint32_t lin_address);

extern void k_mem_invalidate_tlb(uint32_t lin_address);

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

void k_mem_add_block(uint32_t block_address, uint32_t block_size);

uint32_t k_mem_reserve_block(uint32_t size, uint32_t align);

void k_mem_release_block(uint32_t address);

void *k_mem_alloc(uint32_t size, uint32_t align);

uint32_t k_mem_commit(uint32_t address, uint32_t size);

uint32_t k_mem_commit_ctg(uint32_t address, uint32_t size);

void k_mem_defrag();

void k_mem_free(void *memory);  

#endif