#include <stddef.h>
#include <stdarg.h>
#include "proc.h"
#include "thread.h"
#include "../sys/term.h"
#include "../irq/apic.h"
#include "../irq/irq.h"
#include "../k_rng.h"
#include "../mem/mem.h"
#include "../mem/pmap.h"
#include "../rt/alloc.h"
#include "../rt/queue.h"
#include "../rt/mem.h"
// #include "../cont/k_objlist.h"
#include "../rt/atm.h"
// #include "../../libdg/container/dg_slist.h"
// #include "../../libdg/malloc/dg_malloc.h"

// struct dg_slist_t k_proc_processes;
// struct dg_slist_t k_proc_threads;

// union k_proc_thread_list_t *k_proc_thread_lists;
// uint32_t k_proc_thread_id = 0;

// k_atm_spnl_t k_proc_threads_spinlock;
// struct k_cont_objlist_t k_proc_threads;

// struct k_proc_thread_t *k_proc_finished_threads = NULL;

struct k_proc_process_t *k_proc_processes = NULL;
struct k_proc_process_t *k_proc_last_process = NULL;
// struct k_proc_process_t *k_proc_current_process = NULL;



// struct k_proc_thread_t *k_proc_current_thread = NULL;
// struct k_proc_thread_t *k_proc_next_thread = NULL;

struct k_proc_process_t k_proc_scheduler_process;
struct k_proc_process_t *k_proc_active_process;
// struct k_proc_thread_queue_t *k_proc_ready_queue;
struct k_proc_thread_queue_t *k_proc_wait_queue;
// struct k_proc_thread_t k_proc_scheduler_thread;


// struct k_rt_queue_t k_proc_ready_queue;

// k_rt_spnl_t k_proc_ready_queue_spinlock = 0;
// struct k_proc_thread_t *k_proc_ready_queue = NULL;
// struct k_proc_thread_t *k_proc_ready_queue_last = NULL;

// struct k_proc_thread_t *k_proc_finished_list = NULL;

// k_rt_spnl_t k_proc_wait_list_spinlock = 0;
// struct k_proc_thread_t *k_proc_thread_wait_list = NULL;

// struct k_proc_thread_t *k_proc_io_wait_list = NULL;


struct k_proc_thread_t *k_proc_cond_wait_threads = NULL;
struct k_proc_thread_t *k_proc_cond_wait_last_thread = NULL;
k_rt_spnl_t             k_proc_cond_wait_lock = 0;
struct k_proc_thread_t *k_proc_ready_threads = NULL;
struct k_proc_thread_t *k_proc_last_ready_thread = NULL;
k_rt_spnl_t             k_proc_ready_lock;


/* each core will have on of those */
// struct k_proc_thread_t *k_proc_local_delete_list = NULL;
// struct k_proc_thread_t *k_proc_suspended_threads = NULL;

// struct k_proc_thread_queue_t k_proc_thread_queues[];


// struct k_proc_thread_t *k_proc_suspended_queue = NULL;

// k_atm_spnl_t k_proc_spinlock = 0;

// #define K_PROC_THREAD_STACK_PAGE_COUNT 4

// struct k_cpu_tss_t *k_proc_tss = NULL;

struct k_proc_core_state_t k_proc_core_state;
struct k_proc_thread_t *k_proc_cleanup_thread;
// struct k_cpu_seg_desc_t k_proc_gdt[] =
// {
//     K_CPU_SEG_DESC(0x00000000u, 0x000000u, 0, 0, 0, 0, 0),
//     K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
//     K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
//     K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
//     K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
//     K_CPU_SEG_DESC(0x00000000u, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1),
//     K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EOC, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
// };

// struct k_cpu_seg_desc_t k_proc_syscall_ldt;

extern void *k_proc_PreemptThread_a;
extern void *k_proc_StartUserThread_a;
// extern void *k_sys_SysCall;
uint32_t k_proc_page_map;
struct k_proc_shared_data_t *k_proc_shared_data;
uintptr_t k_proc_shared_data_page;
extern void *k_shared_start;
extern void *k_shared_end;
extern void *k_kernel_end2;

extern struct k_irq_desc_t k_irq_idt[K_IRQ_HANDLER_LAST];

void k_proc_Init()
{
    k_proc_shared_data = (struct k_proc_shared_data_t *)K_PROC_SHARED_DATA_ADDRESS;
    k_proc_shared_data->kernel_pmap = k_proc_page_map;
    k_proc_shared_data->gdt[K_PROC_NULL_SEG] = K_CPU_SEG_DESC(0x00000000u, 0x000000u, 0, 0, 0, 0, 0);
    k_proc_shared_data->gdt[K_PROC_R0_DATA_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1);
    k_proc_shared_data->gdt[K_PROC_R0_CODE_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1);
    k_proc_shared_data->gdt[K_PROC_R3_DATA_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_DSEG_TYPE_RW, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1);
    k_proc_shared_data->gdt[K_PROC_R3_CODE_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EO, 3, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1),
    k_proc_shared_data->gdt[K_PROC_REAL_MODE_CODE_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffff, K_CPU_CSEG_TYPE_ER, 0, K_CPU_SEG_GRAN_BYTE, K_CPU_SEG_OP_SIZE_16, 1);
    k_proc_shared_data->gdt[K_PROC_TSS_SEG] = K_CPU_SEG_DESC(0x00000000u, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_proc_shared_data->gdt[K_PROC_R0_C_CODE_SEG] = K_CPU_SEG_DESC(0x00000000u, 0xffffffu, K_CPU_CSEG_TYPE_EOC, 0, K_CPU_SEG_GRAN_4KB, K_CPU_SEG_OP_SIZE_32, 1);

    struct k_proc_thread_t *scheduler_thread = &k_proc_core_state.scheduler_thread;
    // // k_proc_current_process = &k_proc_kernel_process;
    k_proc_core_state.current_thread = scheduler_thread;
    scheduler_thread->process = &k_proc_scheduler_process;
    k_proc_scheduler_process.page_map = k_proc_page_map;

    k_proc_active_process = &k_proc_scheduler_process;

    for(uint32_t bucket_index = 0; bucket_index < K_RT_SMALL_BUCKET_COUNT; bucket_index++)
    {
        scheduler_thread->heap.buckets[bucket_index].first_chunk = NULL;
        scheduler_thread->heap.buckets[bucket_index].last_chunk = NULL;
    }
    scheduler_thread->heap.big_heap = &k_proc_scheduler_process.heap;
    scheduler_thread->current_pmap = k_proc_scheduler_process.page_map;
    scheduler_thread->kernel_stack_offset = 0;
    k_proc_scheduler_process.pid = 0;
    k_proc_scheduler_process.ring = 0;
    k_proc_scheduler_process.heap.spinlock = 0;
    k_proc_scheduler_process.terminal = NULL;

    k_proc_core_state.thread_pool.pages = NULL;
    k_proc_core_state.thread_pool.threads = NULL;
    // k_proc_core_state.thread_pool.threads = k_cont_CreateObjList(sizeof(struct k_proc_thread_t), 512, &scheduler_thread->heap, 1);
    // // k_proc_processes = dg_StackListCreate(sizeof(struct k_proc_process_t), 512);
    // // k_proc_threads = dg_StackListCreate(sizeof(struct k_proc_thread_t), 512);

    k_proc_core_state.tss = k_rt_Malloc(sizeof(struct k_cpu_tss_t), 8);
    k_proc_core_state.tss->ss0 = K_CPU_SEG_SEL(K_PROC_R0_DATA_SEG, 0, 0);
    k_proc_shared_data->gdt[K_PROC_TSS_SEG] = K_CPU_SEG_DESC((uint32_t)k_proc_core_state.tss, 0x67u, K_CPU_SSEG_TYPE_TSS32_AVAL, 0, K_CPU_SEG_GRAN_BYTE, 0, 1);
    k_cpu_Lgdt(k_proc_shared_data->gdt, 8, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0));
    k_cpu_Ltr(K_CPU_SEG_SEL(K_PROC_TSS_SEG, 3, 0));

    k_irq_SetIDTEntry(K_PROC_PREEMPT_THREAD_IRQ_VECTOR, (uintptr_t)&k_proc_PreemptThread_a, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0), 3);
    k_irq_SetIDTEntry(K_PROC_START_USER_THREAD_IRQ_VECTOR, (uintptr_t)&k_proc_StartUserThread_a, K_CPU_SEG_SEL(K_PROC_R0_CODE_SEG, 0, 0), 0);
    k_apic_WriteReg(K_APIC_REG_LVT_TIMER, (k_apic_ReadReg(K_APIC_REG_LVT_TIMER) | (K_PROC_PREEMPT_THREAD_IRQ_VECTOR & 0xff) ) & (0xfff8ffff));
    k_apic_WriteReg(K_APIC_REG_DIV_CONFIG, k_apic_ReadReg(K_APIC_REG_DIV_CONFIG) & (~0xb));
    k_apic_WriteReg(K_APIC_REG_SPUR_INT_VEC, k_apic_ReadReg(K_APIC_REG_SPUR_INT_VEC) | 34);

    // k_proc_ready_queue = k_rt_QueueCreate();

    struct k_proc_thread_t *cleanup_thread = k_proc_CreateKernelThread(k_proc_CleanupThread, NULL);
    cleanup_thread->queue_next = NULL;
    k_proc_ready_threads = NULL;
    k_proc_last_ready_thread = NULL;

    k_proc_core_state.cleanup_thread = cleanup_thread;
    k_proc_core_state.delete_list = NULL;
    k_proc_core_state.delete_list_lock = 0;
}

struct k_proc_process_mem_init_t
{
    struct k_mem_pentry_t *page_dir;

    struct k_mem_pentry_t *page_table;
    uintptr_t cur_page_table_entry;

    uint8_t *page;
    uintptr_t cur_page_entry;
};

void k_proc_MapProcessAddress(struct k_proc_process_mem_init_t *mem_init, uintptr_t linear_address, uintptr_t physical_address)
{
    uint32_t pdir_index = K_MEM_PDIR_INDEX(linear_address);
    uint32_t entry_flags = K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;
    if(!mem_init->page_dir[pdir_index].entry)
    {
        mem_init->page_dir[pdir_index].entry = k_mem_AllocPhysicalPage(0) | entry_flags;

        if(mem_init->cur_page_table_entry)
        {
            k_mem_UnmapLinearAddress((uintptr_t)mem_init->page_table);
        }

        uintptr_t page_table_entry = mem_init->page_dir[pdir_index].entry & K_MEM_PENTRY_ADDR_MASK;
        k_mem_MapLinearAddress((uintptr_t)mem_init->page_table, page_table_entry, entry_flags);
        for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
        {
            mem_init->page_table[entry_index].entry = 0;
        }
        mem_init->cur_page_table_entry = page_table_entry;
    }

    uintptr_t page_table_entry = mem_init->page_dir[pdir_index].entry & K_MEM_PENTRY_ADDR_MASK;

    if(page_table_entry != mem_init->cur_page_table_entry)
    {
        if(mem_init->cur_page_table_entry)
        {
            k_mem_UnmapLinearAddress((uintptr_t)mem_init->page_table);
        }

        k_mem_MapLinearAddress((uintptr_t)mem_init->page_table, page_table_entry, entry_flags);
        mem_init->cur_page_table_entry = page_table_entry;
    }

    uint32_t ptable_index = K_MEM_PTABLE_INDEX(linear_address);

    if(!mem_init->page_table[ptable_index].entry)
    {
        if(physical_address)
        {
            mem_init->page_table[ptable_index].entry = physical_address;
        }
        else
        {
            mem_init->page_table[ptable_index].entry = k_mem_AllocPhysicalPage(0);
        }

        mem_init->page_table[ptable_index].entry |= entry_flags;
    }

    uintptr_t page_entry = mem_init->page_table[ptable_index].entry & K_MEM_PENTRY_ADDR_MASK;

    if(page_entry != mem_init->cur_page_entry)
    {
        if(mem_init->cur_page_entry)
        {
            k_mem_UnmapLinearAddress((uintptr_t)mem_init->page);
        }

        k_mem_MapLinearAddress((uintptr_t)mem_init->page, page_entry, K_MEM_PENTRY_FLAG_READ_WRITE);
        mem_init->cur_page_entry = page_entry;
    }
}

struct k_proc_process_t *k_proc_CreateProcess(void *image, const char *path, const char **args)
{
    struct k_proc_process_t *process = NULL;
    (void)path;
    (void)args;
    if(image)
    {
        struct k_proc_elfh_t *elf_header = (struct k_proc_elfh_t *)image;

        if(elf_header->ident[K_PROC_ELF_MAGIC0_INDEX] == K_PROC_ELF_MAGIC0 &&
           elf_header->ident[K_PROC_ELF_MAGIC1_INDEX] == K_PROC_ELF_MAGIC1 &&
           elf_header->ident[K_PROC_ELF_MAGIC2_INDEX] == K_PROC_ELF_MAGIC2 &&
           elf_header->ident[K_PROC_ELF_MAGIC3_INDEX] == K_PROC_ELF_MAGIC3)
        {
            if(elf_header->type == K_PROC_ELF_TYPE_EXEC)
            {
                struct k_proc_process_mem_init_t mem_init = {};
                uintptr_t page_map_page = k_mem_AllocPhysicalPage(0);

                mem_init.page_dir = k_mem_AllocVirtualRange(4096);
                mem_init.page_table = k_mem_AllocVirtualRange(4096);
                mem_init.page = k_mem_AllocVirtualRange(4096);

                k_mem_MapLinearAddress((uintptr_t)mem_init.page_dir, page_map_page, K_MEM_PENTRY_FLAG_READ_WRITE);

                for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
                {
                    mem_init.page_dir[entry_index].entry = 0;
                }

                uint8_t *program_headers = (uint8_t *)image + elf_header->pheader_offset;
                uint32_t entry_flags = K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;
                for(uint32_t header_index = 0; header_index < elf_header->pheader_ecount; header_index++)
                {
                    struct k_proc_pheader_t *program_header = (struct k_proc_pheader_t *)(program_headers + header_index * elf_header->pheader_esize);
                    uint8_t *segment = (uint8_t *)image + program_header->offset;
                    
                    if(program_header->type == K_PROC_SEGMENT_TYPE_LOAD)
                    {
                        /* round segment start address downwards to closest page */
                        uintptr_t linear_address = program_header->vaddr & K_MEM_4KB_ADDRESS_MASK;
                        /* how many segment bytes we have left to copy */
                        uintptr_t end_linear_address = linear_address + program_header->mem_size;
                        /* how many segment bytes we copied so far */
                        uint32_t copied_segment_bytes = 0;

                        while(linear_address < end_linear_address)
                        {
                            k_proc_MapProcessAddress(&mem_init, linear_address, 0);

                            uint32_t pad_size = 0;
                            // k_sys_TerminalPrintf("%x, %x\n", linear_address, program_header->vaddr);
                            if(linear_address < program_header->vaddr)
                            {
                                /* segment doesn't start exactly at the start of the page, so we need to pad it */
                                pad_size = program_header->vaddr - linear_address;
                                
                                for(uint32_t index = 0; index < pad_size; index++)
                                {
                                    mem_init.page[index] = 0;
                                }
                            }
                            
                            uint32_t copy_size = 0x1000 - pad_size;
                            linear_address += pad_size;

                            if(copied_segment_bytes < program_header->file_size)
                            {
                                /* we still have segment bytes to copy */
                                uint32_t bytes_left = program_header->file_size - copied_segment_bytes;
                                
                                if(copy_size > bytes_left)
                                {
                                    copy_size = bytes_left;
                                }

                                // segment_file_size -= copy_size;
                                k_rt_CopyBytes(mem_init.page + pad_size, segment + copied_segment_bytes, copy_size);
                                copied_segment_bytes += copy_size;
                            }

                            linear_address += copy_size;

                            if(linear_address & 0xfff)
                            {
                                /* segment doesn't end at page end, so we need to pad it */
                                pad_size = 0x1000 - copy_size;

                                for(uint32_t index = 0; index < pad_size; index++)
                                {
                                    mem_init.page[index + copy_size] = 0;
                                }

                                linear_address += pad_size;
                            }
                        }
                    }
                }

                k_proc_MapProcessAddress(&mem_init, K_PROC_SHARED_DATA_ADDRESS, K_PROC_SHARED_DATA_ADDRESS);
                // k_proc_MapProcessAddress(&mem_init, &k_proc_gdt, &k_proc_gdt);

                uintptr_t idt_start = (uintptr_t)&k_irq_idt;
                uintptr_t idt_end = idt_start + sizeof(k_irq_idt);
                // k_sys_TerminalPrintf("%x %x\n", idt_start, idt_end);
                while(idt_start <= idt_end)
                {
                    k_proc_MapProcessAddress(&mem_init, idt_start, idt_start);
                    idt_start += 0x1000;
                }

                uintptr_t shared_start = (uintptr_t)&k_shared_start;
                uintptr_t shared_end = (uintptr_t)&k_shared_end;
                while(shared_start < shared_end)
                {
                    k_proc_MapProcessAddress(&mem_init, shared_start, shared_start);
                    shared_start += 0x1000;
                }

                process = k_rt_Malloc(sizeof(struct k_proc_process_t), 4);

                process->page_map = page_map_page;
                process->streams = NULL;
                process->threads = NULL;
                process->terminal = NULL;
                process->ring = 0;

                if(!k_proc_processes)
                {
                    k_proc_processes = process;
                }
                else
                {
                    k_proc_last_process->next = process;
                    process->prev = process;
                }

                k_proc_last_process = process;

                // process->next = k_proc_processes;
                // k_proc_processes = process;

                struct k_proc_thread_init_t thread_init = {};
                thread_init.entry_point = elf_header->entry;
                thread_init.user_stack_base = (uintptr_t)K_PROC_INIT_STACK_ADDRESS;
                thread_init.ring0_stack_base = thread_init.user_stack_base;
                thread_init.ring0_stack_page = k_mem_AllocPhysicalPage(0);
                thread_init.process = process;
                k_mem_MapLinearAddress(thread_init.ring0_stack_base, thread_init.ring0_stack_page, K_MEM_PENTRY_FLAG_READ_WRITE);
                k_proc_MapProcessAddress(&mem_init, K_PROC_INIT_STACK_ADDRESS, thread_init.ring0_stack_page);

                k_proc_CreateThread(&thread_init);

                process->terminal = k_io_AllocStream();
                k_io_UnblockStream(process->terminal);

                /* FIXME: ugh, this sucks real bad... */
                k_mem_UnmapLinearAddress((uintptr_t)mem_init.page_dir);
                k_mem_UnmapLinearAddress((uintptr_t)mem_init.page_table);
                k_mem_UnmapLinearAddress((uintptr_t)mem_init.page);
                k_mem_FreeVirtualRange(mem_init.page_dir);
                k_mem_FreeVirtualRange(mem_init.page_table);
                k_mem_FreeVirtualRange(mem_init.page);
            }
        }
    }

    return process;
}

struct k_proc_process_t *k_proc_LaunchProcess(const char *path, const char **args)
{
    // return k_proc_CreateProcess(&k_kernel_end2, path, args);
}

uint32_t k_proc_WaitProcess(struct k_proc_process_t *process, uintptr_t *return_value)
{
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();

    if(process && current_process != process)
    {
        k_proc_FocusProcess(process);
        k_proc_WaitThread(process->main_thread, return_value);
        k_proc_FocusProcess(current_process);
        return K_STATUS_OK;
    }
}

uint32_t k_proc_TerminateProcess(uintptr_t return_value)
{
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    struct k_proc_thread_t *main_thread = current_process->main_thread;

    struct k_mem_pentry_t *page_dir = k_mem_AllocVirtualRange(4096);
    struct k_mem_pentry_t *page_table = k_mem_AllocVirtualRange(4096);

    k_mem_MapLinearAddress((uintptr_t)page_dir, current_process->page_map, K_MEM_PENTRY_FLAG_READ_WRITE);

    for(uint32_t page_dir_entry_index = 0; page_dir_entry_index < 1024; page_dir_entry_index++)
    {
        uintptr_t page_table_entry = page_dir[page_dir_entry_index].entry & K_MEM_PENTRY_ADDR_MASK;

        if(page_table_entry)
        {
            k_mem_MapLinearAddress((uintptr_t)page_table, page_table_entry, K_MEM_PENTRY_FLAG_READ_WRITE);

            for(uint32_t page_table_entry_index = 0; page_table_entry_index < 1024; page_table_entry_index++)
            {
                uintptr_t page_entry = page_table[page_table_entry_index].entry & K_MEM_PENTRY_ADDR_MASK;

                if(page_entry)
                {
                    k_mem_FreePhysicalPages(page_entry);
                }
            }

            k_mem_UnmapLinearAddress((uintptr_t)page_table);
            k_mem_FreePhysicalPages(page_table_entry);
        }
    }

    k_mem_UnmapLinearAddress((uintptr_t)page_dir);
    k_mem_FreePhysicalPages(current_process->page_map);

    if(!current_process->prev)
    {
        k_proc_processes = current_process->next;
    }
    else
    {
        current_process->prev->next = current_process->next;
    }

    if(!current_process->next)
    {
        k_proc_last_process = current_process->prev;
    }
    else
    {
        current_process->next->prev = current_process->prev;
    }

    k_rt_Free(current_process);
    k_proc_TerminateThread(return_value);
}

// struct k_proc_process_t *k_proc_GetProcess(uint32_t process_id)
// {
//     (void)process_id;
//     struct k_proc_process_t *process = NULL;
//
//     // process = dg_StackListGetElement(&k_proc_processes, process_id);
//
//     // if(process && process->pid == DG_INVALID_INDEX)
//     // {
//     //     process = NULL;
//     // }
//
//     return process;
// }

struct k_proc_process_t *k_proc_GetCurrentProcess()
{
    return k_proc_core_state.current_thread->process;
}

struct k_proc_process_t *k_proc_GetFocusedProcess()
{
    return k_proc_active_process;
}

void k_proc_FocusProcess(struct k_proc_process_t *process)
{
    k_proc_active_process = process;
}

void k_proc_RunScheduler()
{
    k_proc_core_state.current_thread = &k_proc_core_state.scheduler_thread;
    k_cpu_DisableInterrupts();
    k_proc_YieldThread();

    while(1)
    {
        struct k_proc_thread_t *waiting_thread = NULL;
        struct k_proc_thread_t *resume_thread = NULL;
        struct k_proc_thread_t *last_resume_thread = NULL;
        // k_rt_Xchg32((uint32_t *)&k_proc_cond_wait_list, (uint32_t)NULL, (uint32_t *)&waiting_thread);

        k_rt_SpinLock(&k_proc_cond_wait_lock);
        waiting_thread = k_proc_cond_wait_threads;
        k_proc_cond_wait_threads = NULL;
        k_proc_cond_wait_last_thread = NULL;
        k_rt_SpinUnlock(&k_proc_cond_wait_lock);

        while(waiting_thread)
        {
            struct k_proc_thread_t *next_thread = waiting_thread->queue_next;

            if(*waiting_thread->wait_condition)
            {
                waiting_thread->queue_next = resume_thread;
                resume_thread = waiting_thread;
            }
            else
            {   
                k_proc_QueueWaitingThread(waiting_thread);
            }

            waiting_thread = next_thread;
        }

        if(resume_thread)
        {
            struct k_proc_thread_t *next_resume_thread = resume_thread->queue_next;
            k_proc_QueueReadyThread(resume_thread);
            resume_thread = next_resume_thread;
        }

        k_proc_RunThread(k_proc_core_state.cleanup_thread);
        k_rt_SpinLock(&k_proc_ready_lock);
        struct k_proc_thread_t *next_thread = k_proc_ready_threads;

        if(next_thread)
        {
            k_proc_ready_threads = k_proc_ready_threads->queue_next;
            next_thread->queue_next = NULL;
            k_rt_SpinUnlock(&k_proc_ready_lock);
            k_proc_RunThread(next_thread);
        }
        else
        {
            k_proc_last_ready_thread = NULL;
            k_rt_SpinUnlock(&k_proc_ready_lock);
        }
    }
}

uintptr_t k_proc_CleanupThread(void *data)
{
    (void )data;

    while(1)
    {
        if(k_rt_TrySpinLock(&k_proc_core_state.thread_pool.spinlock))
        {
            uint32_t deleted_count = 0;
            // struct k_proc_thread_t *deleted_thread;
            // k_rt_Xchg32((uint32_t *)&k_proc_core_state.delete_list, (uint32_t)NULL, (uint32_t *)&deleted_thread);

            k_rt_SpinLock(&k_proc_core_state.delete_list_lock);
            struct k_proc_thread_t *deleted_thread = k_proc_core_state.delete_list;
            k_proc_core_state.delete_list = NULL;
            k_rt_SpinUnlock(&k_proc_core_state.delete_list_lock);
            
            while(deleted_thread)
            {
                struct k_proc_thread_t *next_thread = deleted_thread->queue_next;
                k_proc_DestroyThread(deleted_thread);
                deleted_thread = next_thread;
            }
            k_rt_SpinUnlock(&k_proc_core_state.thread_pool.spinlock);
        }
        k_proc_YieldThread();
    }

    return 0;
}

uintptr_t k_proc_IdleThread(void *data)
{
    (void)data;
    
    while(1)
    {
        
    }
}

void k_proc_EnablePreemption()
{
    // if(k_proc_current_thread != &k_proc_scheduler_thread)
    // {
    //     k_cpu_EnableInterrupts();

    //     if(k_apic_ReadReg(K_PROC_THREAD_PREEMPT_ISR_REG) & K_PROC_THREAD_PREEMPT_ISR_BIT)
    //     {
    //         k_proc_Yield();
    //     }
    // }
}

void k_proc_DisablePreemption()
{
    // k_cpu_DisableInterrupts();
}
