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
#define K_MEM_PDIR_SHIFT 22
#define K_MEM_PTABLE_SHIFT 12
#define K_MEM_PENTRY_INDEX_MASK 0x000003ff
#define K_MEM_PENTRY_ADDR_MASK 0xfffff000
#define K_MEM_PTABLE_ADDRESS(entry) ((struct k_mem_pentry_t *)((entry) & K_MEM_PENTRY_ADDR_MASK))
#define K_MEM_PDIR_INDEX(address) ((address >> K_MEM_PDIR_SHIFT) & K_MEM_PENTRY_INDEX_MASK)
#define K_MEM_PTABLE_INDEX(address) ((address >> K_MEM_PTABLE_SHIFT) & K_MEM_PENTRY_INDEX_MASK)

// #define K_MEM_PSTATE_PTABLES_ADDRESS 0xffc00000
#define K_MEM_PSTATE_SELF_DIR_INDEX 0x000003ff
#define K_MEM_PSTATE_SELF_TABLE_FIRST_INDEX 0x000003fd
#define K_MEM_PSTATE_SELF_TABLE_LAST_INDEX 0x000003ff
#define K_MEM_ACTIVE_PSTATE_ADDRESS ((struct k_mem_pstate_t *)0xffffd000) 
#define K_MEM_ACTIVE_PSTATE_DIRS_ADDRESS ((struct k_mem_pentry_t *)0xffffe000)
#define K_MEM_ACTIVE_PSTATE_SELF_TABLE_ADDRESS ((struct k_mem_pentry_t *)0xfffff000)
#define K_MEM_ACTIVE_PSTATE_SELF_TABLE_ENTRY_ADDRESS(dir_index) (K_MEM_ACTIVE_PSTATE_SELF_TABLE_ADDRESS + dir_index)
#define K_MEM_ACTIVE_PSTATE_PTABLE_BASE_ADDRESS (0xffc00000)
#define K_MEM_ACTIVE_PSTATE_PTABLE_ADDRESS(dir_index) ((struct k_mem_pentry_t *)(K_MEM_ACTIVE_PSTATE_PTABLE_BASE_ADDRESS + ((dir_index) << K_MEM_PTABLE_SHIFT)))

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
    K_MEM_PAGING_STATUS_NOT_ALLOWED = 4,
};

struct k_mem_pentry_t
{
    uint32_t entry;
};

// struct k_mem_plist_t
// {
//     struct k_mem_pentry_t entries[1024];
// };
struct k_mem_pstate_t
{
    struct k_mem_pentry_t *phys_page_dir;
    struct k_mem_pentry_t *page_dir;

    /* modifying paging structures get kind of funky when it's done while paging is enabled. While it's
    easy to understand the process used for translation and use the physical address to directly reach
    the paging structures, things get a little less straightforward when every memory access gets
    translated. For example, mapping a 4KB page physical page to a 4KB linear page is just a matter
    of finding which page dir entry the linear address touches, getting the physical address of the page
    table, finding which entry the linear address touches in the page table and then putting the physical 
    address of the page there. This won't work properly when paging is enabled unless identity paging is 
    being used, which likely won't be the case most of the time. 
    
    The problem is that, the physical address being obtained from the page directory, while paging is
    enabled, will be interpreted as a linear address, and will go through all the translation process,
    likely mapping to a non-present physical page, or a physical page that's being for something else
    entirely. This includes the address of the struct itself! 
    
    To properly access the fields of this struct we map its physical pages to the very last page dir,
    and very last page entries in the page table. The struct currently consumes three 4KB pages, and
    is mapped to the addresses 0xffffd000, 0xffffe000 and 0xfffff000. The first page contains the fields
    of the struct, the second contains the page directory and the third page contains the self page table,
    which maps those three pages.

    This self table also has another purpose, which is to map the physical pages used for page tables by other
    directories. This is necessary for modifying entries in those page tables when paging is enabled, because
    accesses to the physical pages that back up those page tables are only allowed if they're also mapped.

    To map a page that holds a page table this table is indexed by the table directory index, and the physical
    address of the page containing a page table is put there. Accesses to the physical page then happen through
    the address 0xffc00000 + (page dir index) << 12. The linear address 0xffc00000 maps to the last page directory,
    which holds the self table, which maps the physical pages containing other page tables. The shifted page dir 
    index is used during translation to obtain an entry in the self table. This entry then maps the linear address
    to the actual physical page containing the page table we want to modify. */
    struct k_mem_pentry_t *self_table;

};

// struct k_mem_balloc_t
// {
//     uint32_t first_page;
//     uint32_t cursor;
//     uint32_t page_count;
// };


struct k_mem_block_t
{
    uint32_t start;
    uint32_t size;
};
struct k_mem_heap_t
{
    uint32_t base;
    uint32_t free_block_cursor;
    struct k_mem_block_t *free_blocks;
};

void k_mem_init();

uint32_t k_mem_alloc_page();

uint32_t k_mem_alloc_pages(uint32_t count);

void k_mem_free_pages(uint32_t page_address);

struct k_mem_pstate_t *k_mem_create_pstate();

void k_mem_destroy_pstate(struct k_mem_pstate_t *pstate);

extern void k_mem_load_pstate(struct k_mem_pstate_t *pstate);

extern uint32_t k_mem_paging_enabled();

void k_mem_enable_paging();

void k_mem_disable_paging();

uint32_t k_mem_map_address(struct k_mem_pstate_t *pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags);

uint32_t k_mem_unmap_address(struct k_mem_pstate_t *pstate, uint32_t lin_address);

extern void k_mem_invalidate_tlb(uint32_t address);

void k_mem_create_heap(struct k_mem_heap_t *heap, uint32_t size);

void k_mem_destroy_heap(struct k_mem_heap_t *heap);

// struct k_mem_balloc_t k_mem_create_balloc(uint32_t page_count);

// void k_mem_destroy_balloc(struct k_mem_balloc_t *balloc);

// void *k_mem_balloc(struct k_mem_balloc_t *balloc, uint32_t size);


#endif