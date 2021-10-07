#include "k_mem_pheap.h"
#include "../k_int.h"

extern struct k_mem_state_t k_mem_state;

void k_mem_SortFreePages()
{
    uint32_t first_free_entry_index = 0;

    for(uint32_t entry_index = k_mem_state.pheap.first_used_page; entry_index <= k_mem_state.pheap.last_used_page; entry_index++)
    {
        /* entry in the used pages list that refers to the current page */
        struct k_mem_uppentry_t *used_entry = k_mem_state.pheap.used_pages + entry_index;

        if(used_entry->flags & K_MEM_PAGE_FLAG_USED || used_entry->pid_index == K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX)
        {
            /* this page is in use, which means it's not present in the list, or this address doesn't represent actual ram */
            continue;
        }

        uint32_t page_address = entry_index << K_MEM_4KB_ADDRESS_SHIFT;
        used_entry->pid_index = first_free_entry_index;
        k_mem_state.pheap.free_pages[first_free_entry_index] = page_address;
        first_free_entry_index++;

        // if(first_used_entry->pid_index != first_free_entry_index)
        // {
        //     /* this page is not in the right place in the free pages list, so we'll need
        //     to swap it with whatever is located there */

        //     uint32_t first_page_address = k_mem_free_pages[first_used_entry->pid_index];
        //     uint32_t second_page_address = k_mem_free_pages[first_free_entry_index];
        //     struct k_mem_uppentry_t *second_used_entry = k_mem_used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(second_page_address);

        //     k_mem_free_pages[first_used_entry->pid_index] = second_page_address;
        //     k_mem_free_pages[first_free_entry_index] = first_page_address;

        //     second_used_entry->pid_index = first_used_entry->pid_index;
        //     first_used_entry->pid_index = first_free_entry_index;
        // }
    }
}

uint32_t k_mem_AllocPage(uint32_t flags)
{
    uint32_t page_address = K_MEM_INVALID_PAGE;

    if(k_mem_state.pheap.free_pages_count)
    {
        k_mem_state.pheap.free_pages_count--;
        page_address = k_mem_state.pheap.free_pages[k_mem_state.pheap.free_pages_count];
        struct k_mem_uppentry_t *used_entry = k_mem_state.pheap.used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
        used_entry->pid_index = 0;
        used_entry->flags = flags | K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD;
    }
    else
    {
        k_int_HaltAndCatchFire();
    }

    return page_address;
}

uint32_t k_mem_AllocPages(uint32_t page_count, uint32_t flags)
{
    uint32_t page_address = K_MEM_INVALID_PAGE;
    uint32_t entries_sorted = 0;

    if(k_mem_state.pheap.free_pages_count && k_mem_state.pheap.free_pages_count >= page_count)
    {
        _try_again:

        /* how many times we'll need to run the loop below if we take into consideration the number of
        pages we want to allocate. For example, imagine we want to allocate 4 pages and there are only
        6 available pages. There's no point in checking pages after the second page, because there won't 
        be enough pages to service the request anyway  */
        uint32_t entry_count = k_mem_state.pheap.free_pages_count - page_count;

        for(uint32_t entry_index = 0; entry_index <= entry_count; entry_index++)
        {
            uint32_t *head_entry = k_mem_state.pheap.free_pages + entry_index;
            uint32_t first_entry = head_entry[0];

            for(uint32_t next_entry_index = 1; next_entry_index < page_count; next_entry_index++)
            {
                uint32_t second_entry = head_entry[next_entry_index];
                /* likely not the ideal thing to do here. Could be more efficient to try
                both combinations, and swap the entries if those point to adjacent pages */
                if(first_entry > second_entry || second_entry - first_entry > K_MEM_4KB_ADDRESS_OFFSET)
                {
                    head_entry = NULL;
                    entry_index += next_entry_index;
                    break; 
                }

                first_entry = second_entry;
            }

            if(head_entry)
            {
                page_address = head_entry[0];
                struct k_mem_uppentry_t *used_head_entry = k_mem_state.pheap.used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
                uint32_t first_page_index = used_head_entry->pid_index;
                uint32_t next_free_page_index = first_page_index + page_count;

                if(next_free_page_index > k_mem_state.pheap.free_pages_count)
                {
                    /* error: the allocator somehow "invented" extra pages */
                }

                uint32_t move_count = k_mem_state.pheap.free_pages_count - next_free_page_index;
                uint32_t move_offset = next_free_page_index - first_page_index;

                /* starts at the first page we're allocating */
                uint32_t *dst_pages = k_mem_state.pheap.free_pages + first_page_index;
                /* starts at the first page after the last page we're allocating */
                uint32_t *src_pages = k_mem_state.pheap.free_pages + next_free_page_index;

                for(uint32_t move_index = 0; move_index < move_count; move_index++)
                {
                    /* we'll move pages from the end of the list to fill the "hole", preserving their order. This is "slow",
                    but allows us to get away with not sorting pages more often */

                    /* we'll move a page into this entry, so offset the used page entry of this page by the amount of positions we'll move it */
                    struct k_mem_uppentry_t *src_page_used_entry = k_mem_state.pheap.used_pages + K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(src_pages[move_index]);
                    src_page_used_entry->pid_index -= move_offset;

                    /* do the movzzzzzzzzzzzzzzzzzz... */
                    dst_pages[move_index] = src_pages[move_index];
                }
                k_mem_state.pheap.free_pages_count -= page_count;

                flags = (flags & ~K_MEM_PAGE_FLAG_HEAD) | K_MEM_PAGE_FLAG_USED;

                if(page_count > 1)
                {
                    /* pages in a contiguous allocation must be pinned, otherwise, they may get swapped out. That will likely cause
                    problems because if a non-head page gets swapped and reused as a head page, deallocation won't function properly.
                    This could be solved by swapping the page flags as well */
                    flags |= K_MEM_PAGE_FLAG_PINNED;
                }

                used_head_entry[0].pid_index = 0;
                used_head_entry[0].flags = flags | K_MEM_PAGE_FLAG_HEAD;

                for(uint32_t entry_index = 1; entry_index < page_count; entry_index++)
                {
                    /* at first, the kernel process owns those pages. The pid value will be later assigned
                    by the process allocating the page */
                    used_head_entry[entry_index].pid_index = 0;
                    used_head_entry[entry_index].flags = flags;
                }

                break;
            }
        }

        if(page_address == K_MEM_INVALID_PAGE && !entries_sorted)
        {
            k_mem_SortFreePages();
            entries_sorted = 1;
            goto _try_again;
        }
    }

    return page_address;
}

uint32_t k_mem_IsValidPage(uint32_t page_address)
{
    if(page_address != K_MEM_INVALID_PAGE)
    {
        return k_mem_state.pheap.used_pages[K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address)].pid_index != K_MEM_USED_SMALL_PAGE_INVALID_ENTRY_INDEX;
    }

    return 0;
}

void k_mem_FreePages(uint32_t page_address)
{
    page_address &= K_MEM_4KB_ADDRESS_MASK;

    if(page_address != K_MEM_INVALID_PAGE)
    {
        uint32_t entry_index = K_MEM_USED_SMALL_PAGE_ENTRY_INDEX(page_address);
        struct k_mem_uppentry_t *used_entry = k_mem_state.pheap.used_pages + entry_index;

        if((used_entry->flags & (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD)) == (K_MEM_PAGE_FLAG_USED | K_MEM_PAGE_FLAG_HEAD))
        {
            do
            {
                used_entry->pid_index = k_mem_state.pheap.free_pages_count;
                used_entry->flags = 0;

                // k_printf("free page %x to %d\n", page_address, k_mem_free_pages_count);

                k_mem_state.pheap.free_pages[k_mem_state.pheap.free_pages_count] = page_address;
                k_mem_state.pheap.free_pages_count++;
                page_address += K_MEM_4KB_ADDRESS_OFFSET;
                entry_index++;

                used_entry = k_mem_state.pheap.used_pages + entry_index;
            }
            while(entry_index <= k_mem_state.pheap.last_used_page &&
                ((used_entry->flags & (K_MEM_PAGE_FLAG_HEAD | K_MEM_PAGE_FLAG_USED)) == K_MEM_PAGE_FLAG_USED));
        }
    }
}