#include "k_pstate.h"
#include "k_phys.h"
#include "../cpu/k_cpu.h"

struct k_mem_pstate_h k_mem_CreatePState() 
{
    struct k_mem_pstate_h pstate = {};

    pstate.self_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    pstate.pdir_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    pstate.ptable_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);

    struct k_mem_pstate_t mapped_pstate;
    /* this will initialize this pstate page directory and page table accordingly. We'll map it to a reserved
    position, destined for the exact purpose of initializing a pstate. */

    /* TODO: this WILL require synchronization */
    k_mem_MapPStateToAddress(&pstate, K_MEM_ACTIVE_PSTATE_INIT_PSTATE_BLOCK_ADDRESS, &mapped_pstate);

    return pstate;
}

void k_mem_MapPState(struct k_mem_pstate_h *pstate, struct k_mem_pstate_t *mapped_pstate)
{
    (void)pstate;
    (void)mapped_pstate;
    /* 4MB aligned block, so we can be sure it'll be contained in a single page
    directory. We're only reserving a portion of the linear memory space here, no
    physical memory is being commited */
    // uint32_t map_address = k_mem_ReserveBlock(0x00400000, 0x00400000);
    // k_mem_MapPStateToAddress(pstate, map_address, mapped_pstate);
}

void k_mem_MapPStateToAddress(struct k_mem_pstate_h *pstate, uint32_t map_address, struct k_mem_pstate_t *mapped_pstate)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(map_address);

    if(!(map_address & 0x003fffff))
    {
        /* only map this pstate if the address is aligned to a 4MB boundary */


        /* page dir entry in the active pstate that will use the page table of the pstate we want to map */
        struct k_mem_pentry_t *active_pdir_entry = K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER + dir_index;

        /* entry in the active pstate page table that will be used to modify the page table of the pstate 
        we want to map */
        struct k_mem_pentry_t *active_ptable_entry = K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER + dir_index;

        active_pdir_entry->entry = pstate->ptable_page | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
        active_ptable_entry->entry = active_pdir_entry->entry;

        /* this address allows us to modify the contents of the physical page that backs the page table
        of the pstate we want to map. We'll need that in case this pstate isn't already initialized */
        struct k_mem_pentry_t *active_pdir_entry_ptable = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER[dir_index].entries;
        k_cpu_InvalidateTLB((uint32_t)active_pdir_entry_ptable);

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));

        /* those addresses are kind of like the magic pointers we use to modify the active pstate contents */
        struct k_mem_pentry_t *mapped_pdir = (struct k_mem_pentry_t *)K_MEM_PSTATE_PDIR_MAP_ADDRESS(map_address);
        struct k_mem_pentry_t *mapped_ptable = (struct k_mem_pentry_t *)K_MEM_PSTATE_PTABLE_MAP_ADDRESS(map_address);
        struct k_mem_pentry_page_t *mapped_pdir_ptables = (struct k_mem_pentry_page_t *)K_MEM_PSTATE_PDIR_PTABLES_MAP_ADDRESS(map_address);

        if((active_pdir_entry_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry & K_MEM_PENTRY_ADDR_MASK) != pstate->ptable_page)
        {
            /* this pstate hasn't been initialized, so we set up the necessary stuff here */

            for(uint32_t entry_index = 0; entry_index < K_MEM_PSTATE_SELF_PTABLE_INDEX; entry_index++)
            {
                active_pdir_entry_ptable[entry_index].entry = 0;    
            }

            uint32_t flags = K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;

            active_pdir_entry_ptable[K_MEM_PSTATE_SELF_PTABLE_INDEX].entry = pstate->self_page | flags;
            active_pdir_entry_ptable[K_MEM_PSTATE_PDIR_PTABLE_INDEX].entry = pstate->pdir_page | flags;
            active_pdir_entry_ptable[K_MEM_PSTATE_PTABLE_PTABLE_INDEX].entry = pstate->ptable_page | flags;

            for(uint32_t entry_index = 0; entry_index <= K_MEM_PSTATE_LAST_USABLE_PDIR_INDEX; entry_index++)
            {
                mapped_pdir[entry_index].entry = 0;    
            }

            mapped_pdir[K_MEM_PSTATE_SELF_PDIR_INDEX].entry = active_pdir_entry->entry;
        }

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));
        // k_mem_invalidate_tlb((uint32_t)mapped_pstate);
        mapped_pstate->page_dir = mapped_pdir;
        mapped_pstate->page_table = mapped_ptable;
        mapped_pstate->page_dir_ptables = mapped_pdir_ptables;

        k_cpu_InvalidateTLB((uint32_t)mapped_pstate->page_dir);
        k_cpu_InvalidateTLB((uint32_t)mapped_pstate->page_table);
        k_cpu_InvalidateTLB((uint32_t)mapped_pstate->page_dir_ptables);
    }
}

void k_mem_UnmapPState(struct k_mem_pstate_t *mapped_pstate)
{
    (void)mapped_pstate;
    // uint32_t dir_index = K_MEM_PDIR_INDEX((uint32_t)mapped_pstate->page_dir_ptables);
    // uint32_t map_address = K_MEM_4MB_ADDRESS_OFFSET * dir_index;

    // if(mapped_pstate->page_table != K_MEM_ACTIVE_PSTATE_PTABLE_MAP_POINTER)
    // {
    //     K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER[dir_index].entry = 0;

    //     k_mem_release_block(map_address);

    //     for(uint32_t page_index = 0; page_index < 1024; page_index++)
    //     {
    //         k_mem_InvalidateTLB(map_address);
    //         map_address += K_MEM_4KB_ADDRESS_OFFSET;
    //     }
    // }
}

void k_mem_DestroyPState(struct k_mem_pstate_t *pstate)
{
    (void)pstate;
}

void k_mem_LoadPState(struct k_mem_pstate_h *pstate)
{
    k_cpu_Lcr3(pstate->pdir_page);
}

uint32_t k_mem_MapAddress(struct k_mem_pstate_t *mapped_pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *pentry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    pentry = mapped_pstate->page_dir + dir_index;

    /* TODO: handle entries marked as not present */

    if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        pentry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(pentry->entry & K_MEM_PENTRY_FLAG_USED) 
        {
            /* this entry already maps an address to a normal page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        /* this the page directory entry references a page table and now we have a pointer to it */
    }
    else
    {
        /* the page directory entry is not being used, so we need to check what size of page we'll be mapping. If it's 
        a big page, all we have to do is point the directory entry to the physical page and set some flags. Otherwise, 
        we'll be mapping a normal page, and we'll need to first allocate a page to hold the directory's page table, map 
        this page, then set up the physical page address and flags we want to actually map. */

        if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE) && (pentry->entry & K_MEM_PENTRY_ADDR_MASK) == 0)
        {
            /* we'll be mapping a normal page and the page directory doesn't have a page table, so we 
            allocate and map a physical page that will hold the page table we need. */

            uint32_t page_table = k_mem_AllocPage(0);

            pentry->entry = page_table | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_PRESENT;
            pentry->entry |= K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
            mapped_pstate->page_table[dir_index].entry = pentry->entry;
            pentry = mapped_pstate->page_dir_ptables[dir_index].entries;

            for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
            {
                k_cpu_InvalidateTLB((uint32_t)(pentry + entry_index));
                pentry[entry_index].entry = 0;
            }

            pentry += table_index;
        }
    }
    
    pentry->entry = phys_address | flags | K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED;
    k_cpu_InvalidateTLB(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}

uint32_t k_mem_IsAddressMapped(struct k_mem_pstate_t *mapped_pstate, uint32_t address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(address);
    struct k_mem_pentry_t *page_entry = NULL;

    page_entry = mapped_pstate->page_dir + dir_index;

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry is used and is mapping a big page */
            return 1;
        }

        /* this entry points to a page table, so we'll check that now */
        uint32_t table_index = K_MEM_PTABLE_INDEX(address);
        page_entry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            return 1;
        }
    }

    /* if the page directory entry is not used, it's guaranteed that the address is not 
    mapped in any way */
    return 0;
}

uint32_t k_mem_IsPageResident(uint32_t address)
{
    // struct k_mem_pstate_p *pstate = K_MEM_ACTIVE_PSTATE;
    uint32_t pdir_index = K_MEM_PDIR_INDEX(address);
    uint32_t ptable_index = K_MEM_PTABLE_INDEX(address);

    struct k_mem_pentry_t *entry = K_MEM_ACTIVE_PSTATE_PDIR_MAP_POINTER + pdir_index;
    
    if(entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(!(entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
        {
            entry = K_MEM_ACTIVE_PSTATE_PDIR_PTABLES_MAP_POINTER[pdir_index].entries + ptable_index;
        }
    }

    return entry->entry & K_MEM_PENTRY_FLAG_PRESENT;
}

void k_mem_MakePageResident(uint32_t address)
{
    if(!k_mem_IsPageResident(address))
    {
        uint32_t page = k_mem_AllocPage(0);
        k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, page, address, K_MEM_PENTRY_FLAG_READ_WRITE);
    }
}

uint32_t k_mem_UnmapAddress(struct k_mem_pstate_t *mapped_pstate, uint32_t lin_address)
{    
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PSTATE_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    page_entry = mapped_pstate->page_dir + dir_index;

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        /* not being used for a big page/doesn't reference a page table */
        return K_MEM_PAGING_STATUS_NOT_PAGED;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        /* this page directory references a page table */

        page_entry = mapped_pstate->page_dir_ptables[dir_index].entries + table_index;

        if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
        {
            /* address not mapped */
            return K_MEM_PAGING_STATUS_NOT_PAGED;
        }
    }

    page_entry->entry = 0;

    k_cpu_InvalidateTLB(lin_address);

    return K_MEM_PAGING_STATUS_OK;
}