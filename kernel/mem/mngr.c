#include "mngr.h"
// #include "../int/int.h"
// #include "../k_term.h"
#include "../rt/atm.h"

struct k_mem_vrlist_t k_mem_virtual_ranges;
struct k_mem_prlist_t k_mem_physical_ranges;
k_rt_spnl_t k_mem_virtual_ranges_spinlock = 0;
k_rt_spnl_t k_mem_physical_ranges_spinlock = 0;

// struct k_mem_pplist_t k_mem_physical_pages;

void *k_mem_AllocVirtualRange(size_t size)
{
    void *memory = NULL;
    size_t request_size = size >> K_MEM_4KB_ADDRESS_SHIFT;

    if(size % K_MEM_4KB_ADDRESS_OFFSET)
    {
        request_size++;
    }

    k_rt_SpinLock(&k_mem_virtual_ranges_spinlock);

    for(uint32_t index = 0; index < k_mem_virtual_ranges.free_count; index++)
    {
        struct k_mem_vrange_t *free_range = k_mem_virtual_ranges.ranges + index;

        if(free_range->size >= request_size)
        {
            // k_printf("found range %x - %x\n", free_range->start, free_range->start + free_range->size);
            memory = (void *)(free_range->start << K_MEM_4KB_ADDRESS_SHIFT);
            uint32_t used_index = k_mem_UsedVirtualRangeIndex(memory);
            struct k_mem_vrange_t *used_range = k_mem_virtual_ranges.ranges + used_index;

            if(free_range->size > request_size)
            {
                /* make space at the used range portion of the list, to fit the newly allocate range */
                for(uint32_t move_index = k_mem_virtual_ranges.range_count; move_index > used_index; move_index--)
                {
                    k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index - 1];
                }

                k_mem_virtual_ranges.range_count++;

                used_range->start = free_range->start;
                used_range->size = request_size;

                free_range->start += request_size;
                free_range->size -= request_size;
            }
            else
            {
                uint32_t range_start = free_range->size;
                uint32_t range_size = free_range->size;
                /* range used completely, so move all ranges back  */
                for(uint32_t move_index = index; move_index < used_index; move_index++)
                {
                    k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index + 1];
                }

                used_range->start = range_start;
                used_range->size = range_size;

                k_mem_virtual_ranges.free_count--;
            }

            break;
        }
    }

    k_rt_SpinUnlock(&k_mem_virtual_ranges_spinlock);

    return (void *)memory;
}

uint32_t k_mem_UsedVirtualRangeIndex(void *range)
{
    // struct k_mem_range_t *used_ranges = k_mem_virtual_ranges.ranges + k_mem_virtual_ranges.free_end;
    uint32_t used_count = k_mem_virtual_ranges.range_count - k_mem_virtual_ranges.free_count;
    uintptr_t range_start = (uintptr_t)range;
    range_start >>= K_MEM_4KB_ADDRESS_SHIFT;

    for(uint32_t index = k_mem_virtual_ranges.free_count; index < k_mem_virtual_ranges.range_count; index++)
    {
        struct k_mem_vrange_t *range = k_mem_virtual_ranges.ranges + index;

        if(range->start >= range_start)
        {
            return index;
        }
    }

    return k_mem_virtual_ranges.range_count;
}

void k_mem_FreeVirtualRange(void *mem)
{
    if(mem)
    {
        k_rt_SpinLock(&k_mem_virtual_ranges_spinlock);
        uint32_t used_index = k_mem_UsedVirtualRangeIndex(mem);
        struct k_mem_vrange_t *used_range = k_mem_virtual_ranges.ranges + used_index;

        // k_printf("try free range %x -- %x (%d) %d\n", used_range->start, used_range->start + used_range->size, used_index, k_mem_virtual_ranges.range_count);

        // k_printf("try to free mem %x - %d - %d\n", (uint32_t)mem, used_index, k_mem_virtual_ranges.range_count);

        if(used_index >= k_mem_virtual_ranges.range_count || used_range->start != ((uintptr_t)mem >> K_MEM_4KB_ADDRESS_SHIFT))
        {
            k_rt_SpinUnlock(&k_mem_virtual_ranges_spinlock);
            return;
        }

        // k_printf("free range %x -- %x\n", used_range->start, used_range->start + used_range->size);

        for(uint32_t free_index = 0; free_index < k_mem_virtual_ranges.free_count; free_index++)
        {
            struct k_mem_vrange_t *free_range = k_mem_virtual_ranges.ranges + free_index;

            // k_printf("test free range %x -- %x\n", free_range->start, free_range->start + free_range->size);
            if(free_range->start > used_range->start)
            {
                // k_printf("free range\n");
                if(free_index)
                {
                    // k_printf("merge start\n");
                    struct k_mem_vrange_t *prev_free_range = free_range - 1;

                    if(used_range->start == prev_free_range->start + prev_free_range->size)
                    {
                        /* used range is adjacent to the previous free range, so merge them. */
                        prev_free_range->size += used_range->size;
                        used_range = prev_free_range;
                    }
                }

                if(used_range->start + used_range->size == free_range->start)
                {
                    // k_printf("merge end\n");
                    /* either the used range or the updated previous free range is adjacent to
                    this range, so merge them. */
                    free_range->start = used_range->start;
                    free_range->size += used_range->size;

                    if(used_range == free_range - 1)
                    {
                        /* used range got merged with the previous free range and the previous range got merged to
                        this range, so we need to move all ranges that come before the used range back one slot */
                        for(uint32_t move_index = free_index - 1; move_index < used_index - 1; move_index++)
                        {
                            k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index + 1];
                        }

                        k_mem_virtual_ranges.free_count--;

                        /* we effectively removed two ranges from the list, and every range before the used range got
                        moved back one position. This means we need to move the remaining used ranges two positions 
                        back. */
                        for(uint32_t move_index = used_index - 1; move_index < k_mem_virtual_ranges.range_count - 2; move_index++)
                        {
                            k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index + 2];
                        }

                        k_mem_virtual_ranges.range_count -= 2;
                    }
                    else
                    {
                        for(uint32_t move_index = used_index; move_index < k_mem_virtual_ranges.range_count - 1; move_index++)
                        {
                            k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index + 1];
                        }

                        k_mem_virtual_ranges.range_count--;
                    }
                }
                else
                {
                    if(used_range == free_range - 1)
                    {
                        /* used range got merged with the previous free range, and is already
                        in its final position. All that's left to do is move the used ranges
                        to fill the slot of the freed used range. */

                        for(uint32_t move_index = used_index; move_index < k_mem_virtual_ranges.range_count - 1; move_index++)
                        {
                            k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index + 1];
                        }

                        k_mem_virtual_ranges.range_count--;
                    }
                    else
                    {
                        // k_printf("used range didn't get merged\n");
                        /* used range didn't get merged, so we need to move forward all all ranges 
                        from the current free range to the used range that comes before the used range
                        we're freeing. */
                        uint32_t used_start = used_range->start;
                        uint32_t used_size = used_range->size;
                        for(uint32_t move_index = used_index; move_index >= free_index + 1; move_index--)
                        {
                            k_mem_virtual_ranges.ranges[move_index] = k_mem_virtual_ranges.ranges[move_index - 1];
                        }

                        k_mem_virtual_ranges.ranges[free_index].start = used_start;
                        k_mem_virtual_ranges.ranges[free_index].size = used_size;
                        k_mem_virtual_ranges.free_count++;
                    }
                }

                break;
            }
        }

        k_rt_SpinUnlock(&k_mem_virtual_ranges_spinlock);
    }
}

void k_mem_SortFreePhysicalPages()
{

    // for(uint32_t range_index = 0; range_index < k_mem_physical_ranges.range_count; range_index++)
    // {
    //     struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + range_index;
    //     for(uint32_t page_index = 0; page_index < range->page_count; page_index++)
    //     {
    //         uint32_t used_index = (range->free_pages[page_index] - range->base_address) >> K_MEM_4KB_ADDRESS_SHIFT;

    //     }
    // }

    // size_t first_free_entry_index = 0;

    // for(size_t entry_index = k_mem_physical_pages.first_used_page; entry_index <= k_mem_physical_pages.last_used_page; entry_index++)
    // {
    //     /* entry in the used pages list that refers to the current page */
    //     struct k_mem_uppentry_t *used_entry = k_mem_physical_pages.used_pages + entry_index;

    //     if(used_entry->flags & K_MEM_PAGE_FLAG_USED || used_entry->pid_index == K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX)
    //     {
    //         /* this page is in use, which means it's not present in the list, or this address doesn't represent actual ram */
    //         continue;
    //     }

    //     uintptr_t page_address = entry_index << K_MEM_4KB_ADDRESS_SHIFT;
    //     used_entry->pid_index = first_free_entry_index;
    //     k_mem_physical_pages.free_pages[first_free_entry_index] = page_address;
    //     first_free_entry_index++;
    // }
}

uintptr_t k_mem_AllocPhysicalPage(uint32_t flags)
{
    uintptr_t page_address = K_MEM_NULL_PAGE;
    // k_cpu_Halt();
    __asm__ volatile ("nop\n");
    k_rt_SpinLock(&k_mem_physical_ranges_spinlock);
    for(uint32_t range_index = 0; range_index < k_mem_physical_ranges.range_count; range_index++)
    {
        struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + range_index;
        // k_printf("alloc from range %d (%d pages)\n", range_index, range->free_count);
        if(range->free_count)
        {
            range->free_count--;
            page_address = range->free_pages[range->free_count];
            // k_printf("alloc page %x\n", page_address);
            // k_cpu_Halt();
            uint32_t page_index = (page_address - range->base_address) >> K_MEM_4KB_ADDRESS_SHIFT;
            range->used_pages[page_index].index = 0;
            range->used_pages[page_index].flags = flags | K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD;
        }
    }
    k_rt_SpinUnlock(&k_mem_physical_ranges_spinlock);

    return page_address;
}

uintptr_t k_mem_AllocPhysicalPages(uint32_t flags, uint32_t count)
{
    uintptr_t page_address = K_MEM_NULL_PAGE;
    uint32_t entries_sorted = 0;

    // if(k_mem_physical_pages.free_pages_count && k_mem_physical_pages.free_pages_count >= count)
    // {
    //     _try_again:

    //     /* how many times we'll need to run the loop below if we take into consideration the number of
    //     pages we want to allocate. For example, imagine we want to allocate 4 pages and there are only
    //     6 available pages. There's no point in checking pages after the second page, because there won't 
    //     be enough pages to service the request anyway  */
    //     size_t entry_count = k_mem_physical_pages.free_pages_count - count;

    //     for(size_t entry_index = 0; entry_index <= entry_count; entry_index++)
    //     {
    //         uintptr_t *head_entry = k_mem_physical_pages.free_pages + entry_index;
    //         uintptr_t first_entry = head_entry[0];

    //         for(uint32_t next_entry_index = 1; next_entry_index < count; next_entry_index++)
    //         {
    //             uintptr_t second_entry = head_entry[next_entry_index];
    //             /* likely not the ideal thing to do here. Could be more efficient to try
    //             both combinations, and swap the entries if those point to adjacent pages */
    //             if(first_entry > second_entry || second_entry - first_entry > K_MEM_4KB_ADDRESS_OFFSET)
    //             {
    //                 head_entry = NULL;
    //                 entry_index += next_entry_index;
    //                 break; 
    //             }

    //             first_entry = second_entry;
    //         }

    //         if(head_entry)
    //         {
    //             page_address = head_entry[0];
    //             struct k_mem_uppentry_t *used_head_entry = k_mem_physical_pages.used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
    //             size_t first_page_index = used_head_entry->pid_index;
    //             size_t next_free_page_index = first_page_index + count;

    //             if(next_free_page_index > k_mem_physical_pages.free_pages_count)
    //             {
    //                 /* error: the allocator somehow "invented" extra pages */
    //             }

    //             size_t move_count = k_mem_physical_pages.free_pages_count - next_free_page_index;
    //             size_t move_offset = next_free_page_index - first_page_index;

    //             /* starts at the first page we're allocating */
    //             uintptr_t *dst_pages = k_mem_physical_pages.free_pages + first_page_index;
    //             /* starts at the first page after the last page we're allocating */
    //             uintptr_t *src_pages = k_mem_physical_pages.free_pages + next_free_page_index;

    //             for(size_t move_index = 0; move_index < move_count; move_index++)
    //             {
    //                 /* we'll move pages from the end of the list to fill the "hole", preserving their order. This is "slow",
    //                 but allows us to get away with not sorting pages more often */

    //                 /* we'll move a page into this entry, so offset the used page entry of this page by the amount of positions we'll move it */
    //                 struct k_mem_uppentry_t *src_page_used_entry = k_mem_physical_pages.used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(src_pages[move_index]);
    //                 src_page_used_entry->pid_index -= move_offset;

    //                 /* do the movzzzzzzzzzzzzzzzzzz... */
    //                 dst_pages[move_index] = src_pages[move_index];
    //             }
    //             k_mem_physical_pages.free_pages_count -= count;

    //             flags = (flags & ~K_MEM_PAGE_FLAG_HEAD) | K_MEM_PAGE_FLAG_USED;

    //             if(count > 1)
    //             {
    //                 /* pages in a contiguous allocation must be pinned, otherwise, they may get swapped out. That will likely cause
    //                 problems because if a non-head page gets swapped and reused as a head page, deallocation won't function properly.
    //                 This could be solved by swapping the page flags as well */
    //                 flags |= K_MEM_PAGE_FLAG_PINNED;
    //             }

    //             used_head_entry[0].pid_index = 0;
    //             used_head_entry[0].flags = flags | K_MEM_PAGE_FLAG_HEAD;

    //             for(size_t entry_index = 1; entry_index < count; entry_index++)
    //             {
    //                 /* at first, the kernel process owns those pages. The pid value will be later assigned
    //                 by the process allocating the page */
    //                 used_head_entry[entry_index].pid_index = 0;
    //                 used_head_entry[entry_index].flags = flags;
    //             }

    //             break;
    //         }
    //     }

    //     if(page_address == K_MEM_INVALID_PAGE && !entries_sorted)
    //     {
    //         k_mem_SortFreePhysicalPages();
    //         entries_sorted = 1;
    //         goto _try_again;
    //     }
    // }

    return page_address;
}

void k_mem_FreePhysicalPages(uintptr_t page)
{
    /* clear possible flags */
    page &= K_MEM_4KB_ADDRESS_MASK;

    if(page != K_MEM_NULL_PAGE)
    {
        k_rt_SpinLock(&k_mem_physical_ranges_spinlock);
        for(uint32_t range_index = 0; range_index < k_mem_physical_ranges.range_count; range_index++)
        {
            struct k_mem_prange_t *range = k_mem_physical_ranges.ranges + range_index;

            uint32_t page_index = (page - range->base_address) >> K_MEM_4KB_ADDRESS_SHIFT;

            if(page >= range->base_address && page_index < range->page_count)
            {
                uint32_t test_flags = K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD;
                struct k_mem_upage_t *used_page = range->used_pages + page_index;
                if((used_page->flags & test_flags) == test_flags)
                {
                    do
                    {
                        range->free_pages[range->free_count] = page;
                        range->used_pages[page_index].index = range->free_count;
                        range->used_pages[page_index].flags = 0;

                        range->free_count++;
                        page_index++;
                        page += K_MEM_4KB_ADDRESS_OFFSET;

                        if(range->free_count > range->page_count)
                        {
                            /* make sure we crash pretty hard if physical pages show up out of nowhere */
                            // k_int_HaltAndCatchFire();
                        }

                        used_page = range->used_pages + page_index;
                    }
                    while((used_page->flags & test_flags) == K_MEM_PAGE_FLAG_USED);

                    k_rt_SpinUnlock(&k_mem_physical_ranges_spinlock);

                    return;
                }
            }
        }
        k_rt_SpinUnlock(&k_mem_physical_ranges_spinlock);
    }
}