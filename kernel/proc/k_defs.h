#ifndef K_PROC_DEFS_H
#define K_PROC_DEFS_H

#include <stdint.h>
#include "../mem/k_mem.h"
#include "../cpu/k_cpu.h"
#include "../../libdg/container/dg_defs.h"
#include "../dsk/k_defs.h"

enum K_PROC_ELF_TYPES
{
    K_PROC_ELF_TYPE_NONE = 0,
    K_PROC_ELF_TYPE_REL = 1,
    K_PROC_ELF_TYPE_EXEC = 2, 
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
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t pheader_offset;
    uint32_t sheader_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t pheader_esize;
    uint16_t pheader_ecount;
    uint16_t sheader_esize;
    uint16_t sheader_ecount;
    uint16_t sheader_str_index;
};

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
    struct k_mem_sheap_t heap;
};

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

// struct k_proc_crit_sec_t
// {
//     k_atm_spnl_t spinlock;
// };

#define K_PROC_KERNEL_PID DG_INVALID_INDEX

#endif