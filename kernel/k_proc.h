#ifndef K_PROC_H
#define K_PROC_H

#include <stdint.h>
#include "mem/k_mem.h"
#include "cpu/k_cpu.h"

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

enum K_PROC_ELF_MACH_TYPES
{
    K_PROC_ELF_MACH_TYPE_NONE = 0,
    K_PROC_ELF_MACH_TYPE_M32 = 1,
    K_PROC_ELF_MACH_TYPE_SPARC = 2,
    K_PROC_ELF_MACH_TYPE_386 = 3,
    K_PROC_ELF_MACH_TYPE_68K = 4,
    K_PROC_ELF_MACH_TYPE_88K = 5,
    K_PROC_ELF_MACH_TYPE_860 = 7,
    K_PROC_ELF_MACH_TYPE_MIPS = 8
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

enum K_PROC_ELF_DATA_TYPES
{
    K_PROC_ELF_DATA_TYPE_NONE = 0,
    K_PROC_ELF_DATA_TYPE_LENDIAN = 1,
    K_PROC_ELF_DATA_TYPE_BENDIAN = 2
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
    uint32_t prog_header_offset;
    uint32_t sect_header_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t prog_header_esize;
    uint16_t prog_header_ecount;
    uint16_t sect_header_esize;
    uint16_t sect_header_ecount;
    uint16_t sect_header_str_index;
};

enum K_PROC_THREAD_FLAGS
{
    K_PROC_THREAD_FLAG_SUSPENDED = 1,
    K_PROC_THREAD_FLAG_IO_BLOCKED = 1 << 1
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

enum K_PROC_THREAD_STATE_REGS
{
    K_PROC_THREAD_STATE_REG_EAX = 0,
    K_PROC_THREAD_STATE_REG_EBX,
    K_PROC_THREAD_STATE_REG_ECX,
    K_PROC_THREAD_STATE_REG_EDX,
    K_PROC_THREAD_STATE_REG_ESI,
    K_PROC_THREAD_STATE_REG_EDI,
    K_PROC_THREAD_STATE_REG_EBP,
    K_PROC_THREAD_STATE_REG_CR3,

    K_PROC_THREAD_STATE_REG_DS,
    K_PROC_THREAD_STATE_REG_ES,
    K_PROC_THREAD_STATE_REG_FS,


    /* those two entries will be filled by the interrupt handler. The point of those entries
    is to give the stack switching code a fixed location from where to read those values */
    // K_PROC_THREAD_STATE_REG_ESP = (K_PROC_THREAD_STATE_BYTES / sizeof(uint32_t)) - 2,
    // K_PROC_THREAD_STATE_REG_SS = K_PROC_THREAD_STATE_REG_ESP + 1
};

struct k_proc_tstate_t
{
    uint32_t state[K_PROC_THREAD_STATE_BYTES];
};

struct k_proc_thread_t
{
    struct k_proc_thread_t *next;   // 0

    uint32_t tid;                   // 4
    uint32_t flags;                 // 8

    uint32_t stack_base;            // 12
    uint32_t entry_point;           // 16
    uint32_t code_seg;              // 20

    uint32_t *start_esp;             // 24
    uint32_t *current_esp;           // 28
};

#define K_PROC_MAX_SEG_DESCS 8
#define K_PROC_CODE_SEG_SEL 0x00000000
#define K_PROC_DATA_SEG_SEL 0x00000008
#define K_PROC_STACK_SEG_SEL 0x00000010

struct k_proc_state_t
{
    struct k_cpu_seg_desc_t ldt[K_PROC_MAX_SEG_DESCS];
    struct k_proc_state_t *next;
    struct k_proc_state_t *prev;
    struct k_mem_pstate_t *pstate;
    struct k_proc_thrd_t *threads;
};

#define K_PROC_KERNEL_PID 0

void k_proc_Init();

uint32_t k_proc_CreateProcess(struct k_mem_pstate_t *pstate, uint32_t start_address, void *image, uint32_t size);

struct k_proc_thread_t *k_proc_CreateThread(void (*thread_fn)(), uint32_t privilege_level);

void k_proc_InitThread(struct k_proc_thread_t *thread, void (*thread_fn)(), uint32_t privilege_level);

void func1();

void func2();

void func3();

void func4();

void func5();

void func6();

void func7();

void k_proc_RunScheduler();

// void k_proc_Yield();

void k_proc_Yield();

extern void k_proc_SwitchToThread(struct k_proc_thread_t *thread);

#endif