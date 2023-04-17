#ifndef PROC_DEFS_H
#define PROC_DEFS_H

#include <stdint.h>
#include "../rt/alloc.h"
#include "../cpu/k_cpu.h"
#include "../rt/atm.h"
// #include "../cont/k_defs.h"
// #include "../dsk/k_defs.h"
#include "../io.h"

/*
===================================================================================
    elf
===================================================================================
*/

enum K_PROC_ELF_TYPES
{
    /* invalid type */
    K_PROC_ELF_TYPE_NONE = 0,
    /* relocatable file */
    K_PROC_ELF_TYPE_REL = 1,
    /* executable */
    K_PROC_ELF_TYPE_EXEC = 2,
    /* shared object */
    K_PROC_ELF_TYPE_DYN = 3,
    K_PROC_ELF_TYPE_CORE = 4,
    K_PROC_ELF_TYPE_LOPROC = 0xff00,
    K_PROC_ELF_TYPE_HIPROC = 0xffff
};

enum K_PROC_ELF_MACHS
{
    K_PROC_ELF_MACH_NONE = 0,
    K_PROC_ELF_MACH_M32 = 1,
    K_PROC_ELF_MACH_SPARC = 2,
    K_PROC_ELF_MACH_386 = 3,
    K_PROC_ELF_MACH_68K = 4,
    K_PROC_ELF_MACH_88K = 5,
    K_PROC_ELF_MACH_860 = 7,
    K_PROC_ELF_MACH_MIPS = 8
};

enum K_PROC_ELF_VERSIONS
{
    K_PROC_ELF_VERSION_NONE = 0,
    K_PROC_ELF_VERSION_CURRENT = 1
};

enum K_PROC_ELF_CLASSES
{
    K_PROC_ELF_CLASS_NONE = 0,
    K_PROC_ELF_CLASS_32 = 1,
    K_PROC_ELF_CLASS_64 = 2,
};

enum K_PROC_ELF_DATA_ENC
{
    K_PROC_ELF_DATA_ENC_NONE = 0,
    K_PROC_ELF_DATA_ENC_LENDIAN = 1,
    K_PROC_ELF_DATA_ENC_BENDIAN = 2
};

#define K_PROC_ELF_MAGIC0 0x7f
#define K_PROC_ELF_MAGIC1 'E'
#define K_PROC_ELF_MAGIC2 'L'
#define K_PROC_ELF_MAGIC3 'F'

#define K_PROC_ELF_MAGIC0_INDEX 0
#define K_PROC_ELF_MAGIC1_INDEX 1
#define K_PROC_ELF_MAGIC2_INDEX 2
#define K_PROC_ELF_MAGIC3_INDEX 3
#define K_PROC_ELF_CLASS_INDEX 4
#define K_PROC_ELF_DATA_INDEX 5
#define K_PROC_ELF_VERSION_INDEX 6
#define K_PROC_ELF_PAD_INDEX 7
#define K_PROC_ELF_IDENT 16

struct k_proc_elfh_t
{
    uint8_t ident[K_PROC_ELF_IDENT];
    /* object file, executable, etc */
    uint16_t type;
    /* SPARC, x386, 68000k, etc */
    uint16_t machine;
    uint32_t version;
    /* virtual address entry point */
    uint32_t entry;
    /* offset to the program header table */
    uint32_t pheader_offset;
    /* offset to the section header table */
    uint32_t sheader_offset;
    uint32_t flags;
    /* size of this header */
    uint16_t header_size;
    /* size of the entries in the program header table */
    uint16_t pheader_esize;
    /* number of entries in the program header table */
    uint16_t pheader_ecount;
    /* size of the entries in the section header table */
    uint16_t sheader_esize;
    /* number of entries in the section header table */
    uint16_t sheader_ecount;
    /* index of the string table in the section header table */
    uint16_t sheader_str_index;
};


enum K_PROC_SECTION_TYPES
{
    /* inactive entry */
    K_PROC_SECTION_TYPE_NULL = 0,
    /* program specific stuff */
    K_PROC_SECTION_TYPE_PROGBITS = 1,
    /* symbol table, may be used for dynamic linking */
    K_PROC_SECTION_TYPE_SYMTAB = 2,
    /* string table. An object file may contain many of those. */
    K_PROC_SECTION_TYPE_STRTAB = 3,
    /* relocation table, with explicit addends */
    K_PROC_SECTION_TYPE_RELA = 4,
    /* symbol hash table */
    K_PROC_SECTION_TYPE_HASH = 5,
    /* holds information for dynamic linking */
    K_PROC_SECTION_TYPE_DYNAMIC = 6,
    /* note */
    K_PROC_SECTION_TYPE_NOTE = 7,
    /* section occupies no space in the file */
    K_PROC_SECTION_TYPE_NOBITS = 8,
    /* relocation table, without explicit addends */
    K_PROC_SECTION_TYPE_REL = 9,
    /* lol */
    K_PROC_SECTION_TYPE_SHLIB = 10,
    /* symbol table, specific for dynamic linking */
    K_PROC_SECTION_TYPE_DYNSYM = 11,
    /* start of process specific semantics */
    K_PROC_SECTION_TYPE_LOPROC = 0x70000000,
    /* end of process specific semantics */
    K_PROC_SECTION_TYPE_HIPROC = 0x7fffffff,
    /* start of reserved user values */
    K_PROC_SECTION_TYPE_LOUSER = 0x80000000,
    /* end of reserved user values */
    K_PROC_SECTION_TYPE_HIUSER = 0xffffffff,
};

enum K_PROC_SECTION_FLAGS
{
    /* section is writable at runtime */
    K_PROC_SECTION_FLAG_WRITE = 1,
    /* section occupies space at runtime */
    K_PROC_SECTION_FLAG_ALLOC = 1 << 1,
    /* section contains code */
    K_PROC_SECTION_FLAG_EXEC = 1 << 2,
    /*  */
    K_PROC_SECTION_FLAG_MASKPROC = 0xf0000000
};

enum K_PROC_SECTION_INDEXES
{
    /* undefined section */
    K_PROC_SECTION_INDEX_UNDEF = 0,

    K_PROC_SECTION_INDEX_LORESERVE = 0xff00,
    K_PROC_SECTION_INDEX_LOPROC = 0xff00,
    K_PROC_SECTION_INDEX_HIPROC = 0xff1f,
    K_PROC_SECTION_INDEX_ABS = 0xfff1,
    K_PROC_SECTION_INDEX_COMMON = 0xfff2,
    K_PROC_SECTION_INDEX_HIRESERVE = 0xffff
};

/* Depending on the section type, 'link' and 'info' have different meanings:

    K_PROC_SECTION_TYPE_DYNAMIC: 'link' holds the section header index of the
    string table used by this section. 'info' holds 0.

    K_PROC_SECTION_TYPE_HASH: 'link' holds the section header index of the
    symbol table it applies to. 'info' holds 0.

    K_PROC_SECTION_TYPE_REL(A): 'link' holds the index for the associated
    symbol table section header. 'info' holds the index of the section header
    to which the relocations apply.

    K_PROC_SECTION_TYPE_SYMTAB/DYNSIM: 'link' holds the section header index
    of the associated string table. 'info' holds
 */
struct k_proc_sheader_t
{
    /* name of the section (indexes into the string table) */
    uint32_t name;
    /* section type */
    uint32_t type;
    /* flags, if applicable */
    uint32_t flags;
    /* memory address of this section, should it exist during runtime */
    uint32_t addr;
    /* start of the section in the file */
    uint32_t offset;
    /* size of the section */
    uint32_t size;
    /* section header index link. Interpretation depends on section type */
    uint32_t link;
    /* extra info. Interpretation depends on section type */
    uint32_t info;
    /* address alignment of the section. The address of the section should be aligned to this value */
    uint32_t addr_align;
    /* some sections, like symbol tables, contain a table of fixed sized entries. This is their size */
    uint32_t entry_size;
};


#define K_PROC_SYMBOL_BINDING_MASK 0xf0
#define K_PROC_SYMBOL_TYPE_MASK 0x0f

enum K_PROC_SYMBOL_BINDINGS
{
    /* local symbols are only visible in the scope of the current object file.
    Local symbols with the same name may exist in many object files */
    K_PROC_SYMBOL_BINDING_LOCAL = 0,
    /* global symbols are visible to all object files, and only one with a specific
    name must exist */
    K_PROC_SYMBOL_BINDING_GLOBAL = 1 << 4,
    /* similar to global symbols, but their definition has lower precedence. This means
    many of those can exist. If there's no definition with global binding, those are used.
    Otherwise, the definition with global binding is used  */
    K_PROC_SYMBOL_BINDING_WEAK = 2 << 4,

    K_PROC_SYMBOL_BINDING_LOPROC = 13 << 4,
    K_PROC_SYMBOL_BINDING_HIPROC = 15 << 4
};

enum K_PROC_SYMBOL_TYPES
{
    /* unspecified type */
    K_PROC_SYMBOL_TYPE_NONE = 0,
    /* data object */
    K_PROC_SYMBOL_TYPE_OBJECT = 1,
    /* function */
    K_PROC_SYMBOL_TYPE_FUNC = 2,
    /* section, used primarily for relocation */
    K_PROC_SYMBOL_TYPE_SECTION = 3,
    K_PROC_SYMBOL_TYPE_FILE = 4,
    K_PROC_SYMBOL_TYPE_LOPROC = 13,
    K_PROC_SYMBOL_TYPE_HIPROC = 15
};



struct k_proc_symbol_t
{
    /* name of the symbol, indexes into the string table referenced by the
    section header */
    uint32_t name;
    /* symbol value */
    uint32_t value;
    /* symbol size */
    uint32_t size;
    /* symbol type and binding */
    uint8_t info;
    /* no defined meaning, always 0 */
    uint8_t other;
    /* index of the section header this symbol refers to */
    uint16_t section;
};


#define K_PROC_REL_SYMBOL_SHIFT 8
#define K_PROC_REL_SYMBOL_INDEX(info) ((info) >> K_PROC_REL_SYMBOL_SHIFT)
#define K_PROC_REL_TYPE_MASK 0xff
#define K_PROC_REL_TYPE(info) ((info) & K_PROC_REL_TYPE_MASK)

enum K_PROC_REL_TYPES
{
    K_PROC_REL_TYPE_386_NONE = 0,
    K_PROC_REL_TYPE_386_32 = 1,
    K_PROC_REL_TYPE_386_PC32 = 2,
    K_PROC_REL_TYPE_386_GOT32 = 3,
    K_PROC_REL_TYPE_386_PLT32 = 4,
    K_PROC_REL_TYPE_386_COPY = 5,
    K_PROC_REL_TYPE_386_GLOB_DAT = 6,
    K_PROC_REL_TYPE_386_JMP_SLOT = 7,
    K_PROC_REL_TYPE_386_RELATIVE = 8,
    K_PROC_REL_TYPE_386_GOTOFF = 9,
    K_PROC_REL_TYPE_386_GOTPC = 10
};

struct k_proc_rel_t
{
    /* offset into the referenced section to apply relocation */
    uint32_t offset;
    /* symbol index and relocation type */
    uint32_t info;
};

struct k_proc_rela_t
{
    /* offset into the referenced section to apply relocation */
    uint32_t offset;
    /* symbol index and relocation type */
    uint32_t info;
    /* explicit addend */
    int32_t addend;
};




enum K_PROC_SEGMENT_TYPES
{
    /* invalid/non used segment */
    K_PROC_SEGMENT_TYPE_NULL = 0,
    /* segment exists during runtime */
    K_PROC_SEGMENT_TYPE_LOAD = 1,
    /* segment contains data for dynamic linking */
    K_PROC_SEGMENT_TYPE_DYNAMIC = 2,
    /* segment specifies an interpreter program to be used when creating the process */
    K_PROC_SEGMENT_TYPE_INTERP = 3,
    /* note */
    K_PROC_SEGMENT_TYPE_NOTE = 4,
    K_PROC_SEGMENT_TYPE_SHLIB = 5,
    K_PROC_SEGMENT_TYPE_PHDR = 6,
    K_PROC_SEGMENT_TYPE_LOPROC = 0x70000000,
    K_PROC_SEGMENT_TYPE_HIPROC = 0x7fffffff
};

enum K_PROC_SEGMENT_FLAGS
{
    K_PROC_SEGMENT_FLAG_EXECUTE = 1,
    K_PROC_SEGMENT_FLAG_WRITE = 1 << 1,
    K_PROC_SEGMENT_FLAG_READ = 1 << 2,
};
struct k_proc_pheader_t
{
    /* segment type */
    uint32_t type;
    /* file offset */
    uint32_t offset;
    /* segment virtual address. This and the file offest should be congruent
    module the max page size */
    uint32_t vaddr;
    /* segment physical address, if applicable */
    uint32_t paddr;
    /* size on file */
    uint32_t file_size;
    /* size on memory. If the memory size is larger
    than the file size, the extra space should be
    filled with 0 */
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
};


// struct k_proc_parsed_elf_t
// {
//     struct k_proc_elfh_t *header;
//     struct k_proc_sheader_t *sheaders;
//     struct k_proc_pheader_t *pheaders;
//     struct k_proc_rel_t *rel_table;
//     struct k_proc_rela_t *rela_table;
//     char *sheader_strtab;
// };


/*
===================================================================================
    thread
===================================================================================
*/

enum K_PROC_THREAD_STATES
{
    // K_PROC_THREAD_STATE_WAITING,
    // K_PROC_THREAD_STATE_IO_BLOCKED,
    K_PROC_THREAD_STATE_COND_WAIT,
    K_PROC_THREAD_STATE_SUSPENDED,
    K_PROC_THREAD_STATE_TERMINATED,

    K_PROC_THREAD_STATE_RUNNING,
    K_PROC_THREAD_STATE_CREATED,
    K_PROC_THREAD_STATE_READY,

    K_PROC_THREAD_STATE_DETACHED,
    K_PROC_THREAD_STATE_INVALID
};

enum K_PROC_THREAD_FLAGS
{
    K_PROC_THREAD_FLAG_DETACHED = 1,
};


#define K_PROC_PREEMPT_THREAD_IRQ_VECTOR 38
#define K_PROC_START_USER_THREAD_IRQ_VECTOR 39
#define K_PROC_THREAD_RETURN_VECTOR 40

#define K_PROC_THREAD_PREEMPT_ISR_REG (K_APIC_REG_IN_SERVICE0 + (K_PROC_THREAD_PREEMPT_VECTOR / 32) * 0x10)
#define K_PROC_THREAD_PREEMPT_ISR_BIT (K_PROC_THREAD_PREEMPT_VECTOR % 32)

#define K_PROC_THREAD_KERNEL_STACK_SIZE 0x1000

#define K_PROC_THREAD_ID_BITS 20
#define K_PROC_THREAD_ID_MASK 0x000fffff
#define K_PROC_THREAD_CORE_BITS 12
#define K_PROC_THREAD_CORE_MASK 0x00000fff
#define K_PROC_INVALID_THREAD_ID K_PROC_THREAD_ID_MASK

#define K_PROC_THREAD_HANDLE(core, id) ((((core) & K_PROC_THREAD_CORE_MASK) << K_PROC_THREAD_CORE_BITS) | ((id) & K_PROC_THREAD_ID_MASK))
#define K_PROC_THREAD_INDEX(handle) ((handle) & K_PROC_THREAD_ID_MASK)
#define K_PROC_THREAD_CORE(handle) (((handle) >> K_PROC_THREAD_CORE_BITS) & K_PROC_THREAD_CORE_MASK)
#define K_PROC_THREAD_VALID(thread) (thread && thread->state != K_PROC_THREAD_STATE_INVALID)

typedef uintptr_t (*k_proc_thread_func_t)(void *data);

struct k_proc_thread_init_t
{
    /* user callback */
    k_proc_thread_func_t entry_point;

    /* process */
    struct k_proc_process_t *process;
    /* physical page allocated by the process to be used during syscalls by the process
    and the kernel. This is here to avoid having the kernel access the process pmap to
    discover the physical page associated with the r0 stack */
    uintptr_t ring0_stack_page;
    /* r0 stack */
    uintptr_t ring0_stack_base;
    /* thread stack, used exclusively by user code */
    uintptr_t user_stack_base;
    /* data to be passed to the user callback */
    uintptr_t user_data;
    // uintptr_t ring;
};

// struct k_proc_thread_regs_t
// {
//     uint32_t regs[];
// };

struct k_proc_thread_t
{
    /* top of the kernel stack */
    uintptr_t *start_sp;                             // 0

    /* current esp after all thread state gets saved during preemption/syscall */
    uintptr_t *current_sp;                           // 4

    /* current page map when the thread got preempted. Most of the time this will
    be the process pmap, but sometimes it may be the kernel pmap if a thread gets
    preempted in the middle of a syscall */
    uintptr_t current_pmap;                           // 8

    /* the kernel stack is mapped both in the kernel address space and in the
    process address space, only to different linear addresses. Adding this value
    to the stack pointer brings the stack from process space to kernel space, and
    subtracting brings it back to process space. */
    uintptr_t kernel_stack_offset;                     // 12

    k_proc_thread_func_t entry_point;                 // 16

    /* thread function return value */
    uintptr_t return_data;                            // 20

    struct k_proc_process_t *process;                 // 24

    // uint32_t id : K_PROC_THREAD_ID_BITS;
    // uint32_t core : K_PROC_THREAD_CORE_BITS;
    uint32_t state : 16;
    uint32_t flags : 16;

    struct k_proc_thread_t *process_next;
    struct k_proc_thread_t *process_prev;
    /* stream this thread is io blocked by */
    struct k_io_stream_t *wait_stream;
    /* thread this thread is waiting on */
    struct k_proc_thread_t *wait_thread;
    /* condition variable this thread is waiting on */
    k_rt_cond_t *wait_condition;
    k_rt_cond_t condition;
    struct k_proc_thread_t *queue_next;
    struct k_rt_sheap_t heap;
};

// struct k_proc_thread_page_header_t
// {
//     struct k_proc_thread_page_t *next;
//     uint32_t count;
// }

#define K_PROC_THREAD_PAGE_FIELDS                     \
    union k_proc_thread_page_t *next;                 \
    uint32_t count;                                   \

#define K_PROC_THREAD_PAGE_THREAD_COUNT  ( (4096 - sizeof(struct {K_PROC_THREAD_PAGE_FIELDS;})) / sizeof(struct k_proc_thread_t))
#define K_PROC_INIT_STACK_ADDRESS 0x2000
#define K_PROC_SHARED_DATA_ADDRESS 0x4000

union k_proc_thread_page_t
{
    struct
    {
        K_PROC_THREAD_PAGE_FIELDS;
        struct k_proc_thread_t threads[K_PROC_THREAD_PAGE_THREAD_COUNT];
    };

    uint8_t bytes[4096];
};

struct k_proc_thread_pool_t
{
    k_rt_spnl_t spinlock;
    union k_proc_thread_page_t *pages;
    struct k_proc_thread_t *threads;
};


// struct k_proc_thread_pool_t
// {
//     k_atm_spnl_t spinlock;
//     struct k_cont_objlist_t threads;
// };

// struct k_proc_detached_thread_list_t
// {
//     struct k_proc_detached_thread_list_t *next;
//     struct k_proc_thread_t *threads;
// };

// struct k_proc_ready_queue_t
// {
//     k_atm_spnl_t spinlock;
//     struct k_proc_thread_t *head;
//     struct k_proc_thread_t *tail;
// };

/*
===================================================================================
    process
===================================================================================
*/

#define K_PROC_MAX_SEG_DESCS 8
#define K_PROC_CODE_SEG_SEL 0x00000000
#define K_PROC_DATA_SEG_SEL 0x00000008
#define K_PROC_STACK_SEG_SEL 0x00000010

#define K_PROC_NULL_SEG 0
#define K_PROC_R0_DATA_SEG 1
#define K_PROC_R0_CODE_SEG 2
#define K_PROC_R3_DATA_SEG 3
#define K_PROC_R3_CODE_SEG 4
#define K_PROC_TSS_SEG 5
#define K_PROC_R0_C_CODE_SEG 6


struct k_proc_process_t
{
    uint32_t page_map;
    struct k_proc_process_t *next;
    struct k_proc_process_t *prev;
    struct k_proc_thread_t *threads;
    struct k_proc_thread_t *last_thread;
    struct k_proc_thread_t *main_thread;
    struct k_io_stream_t *streams;
    struct k_io_stream_t *terminal;
    struct k_rt_bheap_t heap;
    uint32_t generation;
    uint32_t pid : 30;
    uint32_t ring : 2;
};

struct k_proc_phandle_t
{
    uint32_t generation;
    struct k_proc_process_t *process;
};

// struct k_proc_crit_sec_t
// {
//     k_atm_spnl_t spinlock;
// };

#define K_PROC_KERNEL_PID DG_INVALID_INDEX

struct k_proc_core_state_t
{
    /* core tss */
    struct k_cpu_tss_t *tss;                    // 0
    /* currently running thread */
    struct k_proc_thread_t *current_thread;     // 4
    /* thread the core will switch to */
    struct k_proc_thread_t *next_thread;        // 8
    /* scheduler thread state */
    struct k_proc_thread_t scheduler_thread;    // 12
    struct k_proc_thread_t *cleanup_thread;
    struct k_proc_thread_pool_t thread_pool;
    /* core thread delete list */
    struct k_proc_thread_t *delete_list;        // 4
    k_rt_spnl_t             delete_list_lock;
};

struct k_proc_shared_data_t
{
    uint32_t kernel_pmap;
    uint32_t align0;
    struct k_cpu_seg_desc_t gdt[8];
};


#endif
