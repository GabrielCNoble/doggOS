#include <stddef.h>
#include "mem.h"
// #include "k_pmap.h"
// #include "../k_term.h"
#include "../cpu/k_cpu.h"
// #include "../int/int.h"
#include "../proc/defs.h"
#include "../rt/alloc.h"

uint32_t k_mem_total = 0;
uint32_t k_mem_available = 0;
uint32_t k_mem_reserved = 0;

extern struct k_mem_vrlist_t k_mem_virtual_ranges;
extern struct k_mem_prlist_t k_mem_physical_ranges;
extern struct k_proc_process_t k_proc_kernel_process;
extern uint32_t k_proc_page_map;

extern struct k_proc_core_state_t k_proc_core_state;

void *k_mem_BestFitRange(struct k_mem_range_t *ranges, uint32_t *range_count, uint32_t size)
{
    uint32_t count = *range_count;
    struct k_mem_range_t *best_range = NULL;
    uint32_t best_index = 0;
    void *memory = NULL;

    uint32_t size_align = size % 16;

    if(size_align)
    {
        size += 16 - size_align;
    }

    for(uint32_t range_index = 0; range_index < count; range_index++)
    {
        struct k_mem_range_t *range = ranges + range_index;
        
        if(range->size >= size)
        {
            if(!best_range || range->size < best_range->size || range->base < best_range->base)
            {
                best_range = range;
                best_index = range_index;
            }
        }
    }

    if(best_range)
    {
        memory = (void *)((uintptr_t)best_range->base);
        best_range->base += size;
        best_range->size -= size;

        if(!best_range->size)
        {
            for(uint32_t range_index = best_index; range_index < count - 1; range_index++)
            {
                ranges[range_index] = ranges[range_index + 1];
            }

            *range_count = count - 1;
        }

        k_mem_available -= size;
    }

    return memory;
}

void k_mem_MapKernelData(struct k_mem_pentry_t *pdir, uintptr_t address, uint32_t size)
{
    uint32_t flags = K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_READ_WRITE;
    struct k_mem_pentry_t *ptable;

    uintptr_t start_address = (address) >> K_MEM_4KB_ADDRESS_SHIFT;
    uintptr_t end_address = (address + size) >> K_MEM_4KB_ADDRESS_SHIFT;
    uintptr_t init_end_address = end_address;
    // k_printf("before: %x\n", end_address);

    // k_printf("size: %x, computed: %x\n", size, (end_address - start_address) << K_MEM_4KB_ADDRESS_SHIFT);
    if((end_address - start_address) << K_MEM_4KB_ADDRESS_SHIFT < size)
    {
        end_address++;
    }
    // k_printf("after: %x\n", end_address);

    for(uint32_t range_index = 0; range_index < k_mem_virtual_ranges.free_count; range_index++)
    {
        struct k_mem_vrange_t *range = k_mem_virtual_ranges.ranges + range_index;
        uintptr_t range_end = range->start + range->size;

        // if(!range_end)
        // {
        //     range_end--;
        // }


        if(end_address > range->start && start_address < range_end)
        {
            /* memory region intersects with a range, so time to start chopping */
            // k_printf("%x - %x intersects with range %d (%x - %x)\n", start_address, init_end_address, range_index, range->start, range->start + range->size);

            if(start_address <= range->start && end_address >= range_end)
            {
                // k_printf("range covered, drop it\n");
                /* this range is completely covered by the memory region, so we drop it. */
                for(uint32_t copy_index = range_index; copy_index < k_mem_virtual_ranges.free_count - 1; copy_index++)
                {
                    k_mem_virtual_ranges.ranges[copy_index] = k_mem_virtual_ranges.ranges[copy_index + 1];
                }

                k_mem_virtual_ranges.free_count--;

                /* it's guaranteed now that the start address won't intersect with any other ranges, 
                but the end address might, in case it fell outside the now delete range. */
                range_index--;
                continue;
            }
            else
            {
                if(start_address >= range->start)
                {
                    /* start address falls inside the range, so we'll potentially split the
                    range and keep everything that comes before the start address. That can
                    have size 0. */
                    range->size = start_address - range->start;
                }
                else
                {
                    /* start address comes before the start of this range, so we'll set its
                    size to zero to flag to upcoming code this range can be reused if there's
                    a split. */
                    range->size = 0;
                }
                
                if(end_address <= range_end)
                {
                    /* end address falls inside the range, so we'll potentially split the
                    range and keep everything that comes after the end address. That can
                    have size 0. The case where the end address falls outside doesn't need
                    to be handled directly  */
                    uint32_t new_size = range_end - end_address;

                    if(new_size)
                    {
                        if(!range->size)
                        {
                            /* start address didn't generate a split because it's right at 
                            the start of the range, so we can just reuse the same range slot */
                            range->start = range_end - new_size;
                            range->size = new_size;
                        }
                        else
                        {
                            /* both start and end address generated splits. The first split 
                            reused the current range, which means we need to make space for
                            a new range here. */

                            for(uint32_t copy_index = k_mem_virtual_ranges.free_count; copy_index < range_index + 1; copy_index++)
                            {
                                k_mem_virtual_ranges.ranges[copy_index] = k_mem_virtual_ranges.ranges[copy_index - 1];
                            }

                            k_mem_virtual_ranges.free_count++;

                            struct k_mem_vrange_t *new_range = k_mem_virtual_ranges.ranges + range_index + 1;
                            new_range->start = range_end - new_size;
                            new_range->size = new_size;

                            // k_printf("new range %x - %x (%x) created\n", new_range->start, new_range->start + new_range->size, new_range->size);
                        }
                    }
                }

                // k_printf("range %d adjusted to %x - %x\n", range_index, range->start, range->start + range->size);
            }
        }
        else if(end_address <= range->start)
        {
            break;
        }
    }

    start_address = address & K_MEM_4KB_ADDRESS_MASK;
    end_address = address + size;

    while(start_address < end_address)
    {
        uint32_t pdir_index = K_MEM_PDIR_INDEX(start_address);
        uint32_t ptable_index = K_MEM_PTABLE_INDEX(start_address);

        if(!(pdir[pdir_index].entry & K_MEM_PENTRY_FLAG_USED))
        {
            pdir[pdir_index].entry = k_mem_AllocPhysicalPage(K_MEM_PAGE_FLAG_PINNED) | flags;

            ptable = (struct k_mem_pentry_t *)(pdir[K_MEM_PMAP_PDIR_INDEX].entry & K_MEM_4KB_ADDRESS_MASK);
            ptable[pdir_index].entry = pdir[pdir_index].entry;

            ptable = (struct k_mem_pentry_t *)(pdir[pdir_index].entry & K_MEM_4KB_ADDRESS_MASK);
            for(uint32_t index = 0; index < 1024; index++)
            {
                ptable[index].entry = 0;
            }
        }

        ptable = (struct k_mem_pentry_t *)(pdir[pdir_index].entry & K_MEM_4KB_ADDRESS_MASK);

        if(!(ptable[ptable_index].entry & K_MEM_PENTRY_FLAG_USED))
        {
            ptable[ptable_index].entry = start_address | flags;
        }

        start_address += K_MEM_4KB_ADDRESS_OFFSET;
    }
}

void k_mem_Init(struct k_mem_range_t *ranges, uint32_t range_count)
{
    /*
        align found memory ranges to page boundaries
    */
    for(uint32_t range_index = 0; range_index < range_count; range_index++)
    {
        struct k_mem_range_t *range = ranges + range_index;

        uint32_t align = (uint32_t)range->base % 4096;

        if(align)
        {
            align = 4096 - align;
        }

        range->base += align;
        range->size -= align;

        k_mem_total += (uint32_t)range->size;
    }

    k_mem_available = k_mem_total;
    /*
        virtual memory ranges
    */

    // uint32_t *p0 = 0x112345;
    // uint32_t *p1 = 0x012345;
    // k_cpu_Halt();
    // *p0 = 5;
    // if(*p1 == 5)
    // {
    //     k_printf("well, fuck...\n");
    // }
    // else
    // {
    //     k_printf("shit\n");
    // }
    // k_cpu_Halt();

    /* how many vranges fit in a page */
    uint32_t virtual_ranges_per_page = 4096 / sizeof(struct k_mem_vrange_t);
    /* how many bytes of virtual address a page full of vranges cover, assuming each
    range covers a page. This is the worst case scenario. Most of the time a range
    should cover more than a single page */
    uint32_t covered_virtual_bytes_block_size = 4096 * virtual_ranges_per_page + 4096;

    /* how many ranges do we need to cover the whole 4G address space, considering
    the memory overhead for the ranges */
    uint32_t vrange_block_count = 0xffffffff / covered_virtual_bytes_block_size;
    

    if(0xffffffff % covered_virtual_bytes_block_size)
    {
        vrange_block_count++;
    }

    uint32_t vrange_bytes = vrange_block_count * 4096;
    k_mem_virtual_ranges.free_count = 1;
    k_mem_virtual_ranges.range_count = 1;
    k_mem_virtual_ranges.ranges = k_mem_BestFitRange(ranges, &range_count, vrange_bytes);

    if(!k_mem_virtual_ranges.ranges)
    {
        // k_int_HaltAndCatchFire();
    }

    k_mem_virtual_ranges.ranges[0].start = 0x00001000;
    k_mem_virtual_ranges.ranges[0].size = (0xffffffff - 0x1000) >> K_MEM_4KB_ADDRESS_SHIFT;
    

    /*
        physical memory ranges
    */

    k_mem_physical_ranges.ranges = k_mem_BestFitRange(ranges, &range_count, sizeof(struct k_mem_prange_t) * range_count);
    k_mem_physical_ranges.range_count = range_count;

    if(!k_mem_physical_ranges.ranges)
    {
        // k_int_HaltAndCatchFire();
    }


    uint32_t header_pair_size = sizeof(uint32_t) + sizeof(struct k_mem_upage_t);
    uint32_t header_pair_count = k_mem_available / (header_pair_size + 4096);
    // k_printf("available: %d\n", k_mem_available);
    uintptr_t page_headers = (uintptr_t)k_mem_BestFitRange(ranges, &range_count, header_pair_count * header_pair_size);
    // k_printf("%x -- %x\n", page_headers, page_headers + header_pair_count * header_pair_size);

    // uintptr_t *p = page_headers + ((header_pair_count - 1) * header_pair_size);
    // *p = 5;
    if(page_headers)
    {
        for(uint32_t range_index = 0; range_index < range_count; range_index++)
        {
            struct k_mem_range_t *range = ranges + range_index;
            uint32_t range_start = (uint32_t)range->base;
            uint32_t range_size = (uint32_t)range->size;
            struct k_mem_prange_t *prange = k_mem_physical_ranges.ranges + range_index;
            uint32_t align = range_start % 4096;
            if(align)
            {
                align = 4096 - align;
                range_start += align;
                range_size -= align;
            }

            uint32_t page_count = range_size / 4096;
            // k_printf("%d\n", range_size);
            uintptr_t base_address = (uintptr_t)range_start;
            prange->base_address = base_address;
            prange->free_pages = (uint32_t *)page_headers;
            page_headers += sizeof(uint32_t) * page_count;
            prange->used_pages = (struct k_mem_upage_t *)page_headers;
            page_headers += sizeof(struct k_mem_upage_t) * page_count;
            prange->free_count = page_count;
            prange->page_count = page_count;

            for(uint32_t page_index = 0; page_index < page_count; page_index++)
            {
                prange->free_pages[page_index] = base_address;
                prange->used_pages[page_index].index = page_index;
                prange->used_pages[page_index].flags = 0;
                base_address += K_MEM_4KB_ADDRESS_OFFSET;
            }
        }
    }
    else
    {
        /* there isn't a big enough contiguous block of memory to hold all headers, so we'll need to split them */

        // for(uint32_t range_index = 0; range_index < range_count; range_index++)
        // {
        //     struct k_mem_range_t *range = ranges + range_index;
        //     struct k_mem_prange_t *prange = k_mem_physical_ranges.ranges + k_mem_physical_ranges.range_count;

        //     /* how many page headers we need to cover the available physical memory, considering the
        //     memory overhead for the header themselves */
        //     uint32_t header_pair_count = (uint32_t)range->size / covered_physical_bytes_block_size;

        //     if(header_pair_count)
        //     {
        //         if((uint32_t)range->size % covered_physical_bytes_block_size)
        //         {
        //             header_pair_count++;
        //         }

        //         uint32_t phys_page_header_bytes = header_pair_count * (sizeof(struct k_mem_upage_t) + sizeof(uint32_t));
        //         void *page_headers = k_mem_BestFitRange(ranges, &range_count, phys_page_header_bytes);
        //     }
        // }
    }

    uint32_t flags = K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE;
    struct k_mem_pentry_t *pdir = (struct k_mem_pentry_t *)k_mem_AllocPhysicalPage(K_MEM_PAGE_FLAG_PINNED);
    struct k_mem_pentry_t *ptable = (struct k_mem_pentry_t *)k_mem_AllocPhysicalPage(K_MEM_PAGE_FLAG_PINNED);

    
    for(uint32_t index = 0; index < 1024; index++)
    {
        pdir[index].entry = 0;
        ptable[index].entry = 0;
    }

    pdir[K_MEM_PMAP_PDIR_INDEX].entry = ((uintptr_t)ptable) | flags;
    ptable[K_MEM_PMAP_PDIR_INDEX].entry = ((uintptr_t)pdir) | flags;
    ptable[K_MEM_PMAP_PTABLE_INDEX].entry = ((uintptr_t)ptable) | flags;    

    // pdir[0].entry = ((uintptr_t)low_mem_ptable) | flags;
    // ptable[0].entry = ((uintptr_t)low_mem_ptable) | flags;
    
    /* map the low 640K */
    k_mem_MapKernelData(pdir, 0x00001000, 0x00100000 - 0x00001000);
    k_mem_MapKernelData(pdir, (uintptr_t)k_mem_virtual_ranges.ranges, vrange_bytes);
    k_mem_MapKernelData(pdir, (uintptr_t)k_mem_physical_ranges.ranges, sizeof(struct k_mem_prange_t) * k_mem_physical_ranges.range_count);
    
    for(uint32_t range_index = 0; range_index < k_mem_physical_ranges.range_count; range_index++)
    {
        struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + range_index;
        k_mem_MapKernelData(pdir, (uintptr_t)range->free_pages, sizeof(uint32_t) * range->page_count);
        k_mem_MapKernelData(pdir, (uintptr_t)range->used_pages, sizeof(struct k_mem_upage_t) * range->page_count);
    }
    // struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + k_mem_physical_ranges.range_count - 1;

    // uintptr_t p = page_headers + ((header_pair_count - 1) * header_pair_size);
    // uintptr_t p = (uintptr_t)(range->used_pages + range->page_count - 1);
    // uint32_t dir_index = K_MEM_PDIR_INDEX(p);
    // uint32_t tab_index = K_MEM_PTABLE_INDEX(p);
    // ptable = (struct k_mem_pentry_t *)(ptable[dir_index].entry & K_MEM_4KB_ADDRESS_MASK);
    // *p = 5;

    // k_printf("%x %x %d %d\n", pdir[dir_index].entry, ptable[tab_index].entry, dir_index, tab_index);

    k_cpu_Lcr3((uint32_t)pdir);
    k_cpu_EnablePaging();


    
    // k_mem_MapLinearAddress(0xa0000, 0xa0000, K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS);
    k_mem_virtual_ranges.range_count = k_mem_virtual_ranges.free_count;
    // struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + k_mem_physical_ranges.range_count - 1;
    // uintptr_t page = range->free_pages[range->free_count - 1];
    // k_printf("%d\n", range->page_count); 
    // struct k_mem_upage_t *used = range->used_pages + range->page_count - 1;
    // used->index = 15;
    // k_printf("%x %d\n", used, used->index);
    // k_cpu_Halt();    

    k_proc_page_map = (uint32_t)pdir;
}
