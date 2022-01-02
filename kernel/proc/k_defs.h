#ifndef K_PROC_DEFS_H
#define K_PROC_DEFS_H

#include <stdint.h>
#include "../mem/k_mem.h"
#include "../cpu/k_cpu.h"
#include "../../libdg/container/dg_defs.h"
#include "../dsk/k_defs.h"

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
    /* 32 bit, 64 bit, etc */
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
    K_PROC_THREAD_STATE_RUNNING = 1,
    K_PROC_THREAD_STATE_READY = 1 << 1,
    K_PROC_THREAD_STATE_IO_BLOCKED = 1 << 2,
    K_PROC_THREAD_STATE_SUSPENDED = 1 << 3,
};

/* if there's a stack change, the cpu will push 20 bytes worth of state onto the stack, 
and 12 otherwise, so we allocate for the worst case */
#define K_PROC_THREAD_STATE_PUSHED_BYTES (sizeof(uint32_t) * 5)

/* eax, ebx, ecx, edx, eip, eflags, esp, ebp, edi, esi, cr3, ss, cs, ds, es, fs, gs */
#define K_PROC_THREAD_STATE_REG_BYTES (sizeof(uint32_t) * 17)

/* some extra space so the thread swich function has space to store its stuff */
#define K_PROC_THREAD_STATE_EXTRA_BYTES (sizeof(uint32_t) * 10)


#define K_PROC_THREAD_STATE_BYTES (K_PROC_THREAD_STATE_PUSHED_BYTES +  \
                                   K_PROC_THREAD_STATE_REG_BYTES + \
                                   K_PROC_THREAD_STATE_EXTRA_BYTES)

#define K_PROC_THREAD_STATE_LAST_INDEX ((K_PROC_THREAD_STATE_BYTES / sizeof(uint32_t)) - 1)

#define K_PROC_THREAD_PREEMPT_VECTOR 38
#define K_PROC_THREAD_PREEMPT_ISR_REG (K_APIC_REG_IN_SERVICE0 + (K_PROC_THREAD_PREEMPT_VECTOR / 32) * 0x10)
#define K_PROC_THREAD_PREEMPT_ISR_BIT (K_PROC_THREAD_PREEMPT_VECTOR % 32)

// enum K_PROC_THREAD_STATE_REGS
// {
//     K_PROC_THREAD_STATE_REG_EAX = 0,
//     K_PROC_THREAD_STATE_REG_EBX,
//     K_PROC_THREAD_STATE_REG_ECX,
//     K_PROC_THREAD_STATE_REG_EDX,
//     K_PROC_THREAD_STATE_REG_ESI,
//     K_PROC_THREAD_STATE_REG_EDI,
//     K_PROC_THREAD_STATE_REG_EBP,
//     K_PROC_THREAD_STATE_REG_CR3,

//     K_PROC_THREAD_STATE_REG_DS,
//     K_PROC_THREAD_STATE_REG_ES,
//     K_PROC_THREAD_STATE_REG_FS,
// };

// struct k_proc_tstate_t
// {
//     uint32_t state[K_PROC_THREAD_STATE_BYTES];
// };

struct k_proc_thread_t
{
    struct k_proc_thread_t *next;
    struct k_proc_thread_t *prev;
    struct k_proc_process_t *process;

    uint32_t tid;
    uint32_t state;

    uintptr_t stack_base;
    uintptr_t entry_point;
    uint32_t code_seg;

    uintptr_t *start_esp;
    uintptr_t *current_esp;
    uintptr_t page_dir;

    struct k_proc_thread_t *queue_link;
    struct k_proc_thread_t *process_next;
    struct k_mem_sheap_t heap;
};

#define K_PROC_THREAD_LIST_BUFFER_OFFSET                                    \
    offsetof(struct {                                                       \
        union k_proc_thread_list_t *next_list;                              \
        struct k_proc_thread_t *next_thread;                                \
        struct k_proc_thread_t threads[0];                                  \
    }, threads)                                                             \

union k_proc_thread_list_t 
{
    struct
    { 
        union k_proc_thread_list_t *next_list;
        struct k_proc_thread_t *next_thread;
        struct k_proc_thread_t threads[(4096 - K_PROC_THREAD_LIST_BUFFER_OFFSET) / sizeof(struct k_proc_thread_t)]; 
    };

    uint8_t bytes[4096];
};

/*
===================================================================================
    process
===================================================================================
*/

#define K_PROC_MAX_SEG_DESCS 8
#define K_PROC_CODE_SEG_SEL 0x00000000
#define K_PROC_DATA_SEG_SEL 0x00000008
#define K_PROC_STACK_SEG_SEL 0x00000010
struct k_proc_process_t
{
    struct k_proc_process_t *next;
    struct k_proc_process_t *prev;
    struct k_proc_thread_t *threads;
    struct k_proc_thread_t *last_thread;
    struct k_mem_page_map_h page_map;    
    struct k_mem_bheap_t heap;
    uint32_t pid;
};

union k_proc_process_list_t
{
    struct
    {

    };
};

// struct k_proc_crit_sec_t
// {
//     k_atm_spnl_t spinlock;
// };

#define K_PROC_KERNEL_PID DG_INVALID_INDEX

#endif