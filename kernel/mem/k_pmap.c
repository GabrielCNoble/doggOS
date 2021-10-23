#include "k_pmap.h"
#include "k_phys.h"
#include "../cpu/k_cpu.h"

struct k_mem_page_map_h k_mem_CreatePageMap() 
{
    struct k_mem_page_map_h page_map_handle = {};

    page_map_handle.self_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    page_map_handle.pdir_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);
    page_map_handle.ptable_page = k_mem_AllocPage(K_MEM_PAGE_FLAG_PINNED);

    struct k_mem_page_map_t page_map;
    /* this will initialize this pstate page directory and page table accordingly. We'll map it to a reserved
    position, destined for the exact purpose of initializing a pstate. */

    /* TODO: this WILL require synchronization */
    k_mem_MapPageMapToAddress(&page_map_handle, K_MEM_ACTIVE_PAGE_MAP_INIT_PAGE_MAP_BLOCK_ADDRESS, &page_map);

    return page_map_handle;
}

void k_mem_MapPageMap(struct k_mem_page_map_h *page_map_handle, struct k_mem_page_map_t *page_map)
{
    (void)page_map_handle;
    (void)page_map;
    /* 4MB aligned block, so we can be sure it'll be contained in a single page
    directory. We're only reserving a portion of the linear memory space here, no
    physical memory is being commited */
    // uint32_t map_address = k_mem_ReserveBlock(0x00400000, 0x00400000);
    // k_mem_MapPStateToAddress(pstate, map_address, mapped_pstate);
}

void k_mem_MapPageMapToAddress(struct k_mem_page_map_h *page_map_handle, uint32_t map_address, struct k_mem_page_map_t *page_map)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(map_address);

    if(!(map_address & 0x003fffff))
    {
        /* only map this pstate if the address is aligned to a 4MB boundary */


        /* page dir entry in the active pstate that will use the page table of the pstate we want to map */
        struct k_mem_pentry_t *active_pdir_entry = K_MEM_ACTIVE_PAGE_MAP_PDIR_MAP_POINTER + dir_index; 

        /* entry in the active pstate page table that will be used to modify the page table of the pstate 
        we want to map */
        struct k_mem_pentry_t *active_ptable_entry = K_MEM_ACTIVE_PAGE_MAP_PTABLE_MAP_POINTER + dir_index;
        uint32_t entry_flags = K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;
        active_pdir_entry->entry = page_map_handle->ptable_page | entry_flags;
        active_ptable_entry->entry = active_pdir_entry->entry;

        /* this address allows us to modify the contents of the physical page that backs the page table
        of the pstate we want to map. We'll need that in case this pstate isn't already initialized */
        struct k_mem_pentry_t *active_pdir_entry_ptable = K_MEM_ACTIVE_PAGE_MAP_PDIR_PTABLES_MAP_POINTER[dir_index].entries;
        k_cpu_InvalidateTLB((uint32_t)active_pdir_entry_ptable);

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));

        /* those addresses are kind of like the magic pointers we use to modify the active pstate contents */
        struct k_mem_pentry_t *mapped_pdir = (struct k_mem_pentry_t *)K_MEM_PAGE_MAP_PDIR_MAP_ADDRESS(map_address);
        struct k_mem_pentry_t *mapped_ptable = (struct k_mem_pentry_t *)K_MEM_PAGE_MAP_PTABLE_MAP_ADDRESS(map_address);
        struct k_mem_pentry_page_t *mapped_pdir_ptables = (struct k_mem_pentry_page_t *)K_MEM_PAGE_MAP_PDIR_PTABLES_MAP_ADDRESS(map_address);

        if((active_pdir_entry_ptable[K_MEM_PAGE_MAP_PTABLE_PTABLE_INDEX].entry & K_MEM_PENTRY_ADDR_MASK) != page_map_handle->ptable_page)
        {
            /* this pstate hasn't been initialized, so we set up the necessary stuff here */

            for(uint32_t entry_index = 0; entry_index < K_MEM_PAGE_MAP_SELF_PTABLE_INDEX; entry_index++)
            {
                active_pdir_entry_ptable[entry_index].entry = 0;    
            }

            entry_flags = K_MEM_PENTRY_FLAG_PRESENT | K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PAGE_CACHE_DISABLE | K_MEM_PENTRY_FLAG_USER_MODE_ACCESS;

            active_pdir_entry_ptable[K_MEM_PAGE_MAP_SELF_PTABLE_INDEX].entry = page_map_handle->self_page | entry_flags;
            active_pdir_entry_ptable[K_MEM_PAGE_MAP_PDIR_PTABLE_INDEX].entry = page_map_handle->pdir_page | entry_flags;
            active_pdir_entry_ptable[K_MEM_PAGE_MAP_PTABLE_PTABLE_INDEX].entry = page_map_handle->ptable_page | entry_flags;

            for(uint32_t entry_index = 0; entry_index <= K_MEM_PAGE_MAP_LAST_USABLE_PDIR_INDEX; entry_index++)
            {
                mapped_pdir[entry_index].entry = 0;    
            }

            mapped_pdir[K_MEM_PAGE_MAP_SELF_PDIR_INDEX].entry = active_pdir_entry->entry;
        }

        // mapped_pstate = (struct k_mem_pstate_p *)(map_address + (K_MEM_4KB_ADDRESS_OFFSET * K_MEM_PSTATE_SELF_PTABLE_INDEX));
        // k_mem_invalidate_tlb((uint32_t)mapped_pstate);
        page_map->page_dir = mapped_pdir;
        page_map->page_table = mapped_ptable;
        page_map->page_dir_ptables = mapped_pdir_ptables;

        k_cpu_InvalidateTLB((uint32_t)page_map->page_dir);
        k_cpu_InvalidateTLB((uint32_t)page_map->page_table);
        k_cpu_InvalidateTLB((uint32_t)page_map->page_dir_ptables);
    }
}

void k_mem_UnmapPState(struct k_mem_page_map_t *page_map)
{
    (void)page_map;
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

void k_mem_DestroyPageMap(struct k_mem_page_map_t *page_map)
{
    (void)page_map;
}

void k_mem_LoadPageMap(struct k_mem_page_map_h *page_map)
{
    k_cpu_Lcr3(page_map->pdir_page);
}

uint32_t k_mem_MapAddressOnPageMap(struct k_mem_page_map_t *page_map, uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *pentry = NULL;

    if(dir_index == K_MEM_PAGE_MAP_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    pentry = page_map->page_dir + dir_index;

    /* TODO: handle entries marked as not present */

    if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry already maps an address to a big page */
            return K_MEM_PAGING_STATUS_ALREADY_USED;
        }

        pentry = page_map->page_dir_ptables[dir_index].entries + table_index;

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
            page_map->page_table[dir_index].entry = pentry->entry;
            pentry = page_map->page_dir_ptables[dir_index].entries;

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

uint32_t k_mem_MapAddress(uint32_t phys_address, uint32_t lin_address, uint32_t flags)
{
    return k_mem_MapAddressOnPageMap(&K_MEM_ACTIVE_MAPPED_PAGE_MAP, phys_address, lin_address, flags);
}

uint32_t k_mem_IsAddressMapped(struct k_mem_page_map_t *page_map, uint32_t address)
{
    uint32_t dir_index = K_MEM_PDIR_INDEX(address);
    struct k_mem_pentry_t *page_entry = NULL;

    page_entry = page_map->page_dir + dir_index;

    if(page_entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            /* this entry is used and is mapping a big page */
            return 1;
        }

        /* this entry points to a page table, so we'll check that now */
        uint32_t table_index = K_MEM_PTABLE_INDEX(address);
        page_entry = page_map->page_dir_ptables[dir_index].entries + table_index;

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

    struct k_mem_pentry_t *entry = K_MEM_ACTIVE_PAGE_MAP_PDIR_MAP_POINTER + pdir_index; 
    
    if(entry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(!(entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
        {
            entry = K_MEM_ACTIVE_PAGE_MAP_PDIR_PTABLES_MAP_POINTER[pdir_index].entries + ptable_index;
        }
    }

    return entry->entry & K_MEM_PENTRY_FLAG_PRESENT;
}

void k_mem_MakePageResident(uint32_t address)
{
    if(!k_mem_IsPageResident(address))
    {
        uint32_t page = k_mem_AllocPage(0);
        k_mem_MapAddress(page, address, K_MEM_PENTRY_FLAG_READ_WRITE);
    }
}

uint32_t k_mem_UnmapAddress(struct k_mem_page_map_t *page_map, uint32_t lin_address)
{    
    uint32_t dir_index = K_MEM_PDIR_INDEX(lin_address);
    uint32_t table_index = K_MEM_PTABLE_INDEX(lin_address);
    struct k_mem_pentry_t *page_entry = NULL;

    if(dir_index == K_MEM_PAGE_MAP_SELF_PDIR_INDEX)
    {
        return K_MEM_PAGING_STATUS_NOT_ALLOWED;
    }

    page_entry = page_map->page_dir + dir_index;

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        /* not being used for a big page/doesn't reference a page table */
        return K_MEM_PAGING_STATUS_NOT_PAGED;
    }

    if(!(page_entry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        /* this page directory references a page table */

        page_entry = page_map->page_dir_ptables[dir_index].entries + table_index;

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