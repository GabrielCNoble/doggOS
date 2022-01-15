#include "pmap.h"
#include "mngr.h"
#include "../cpu/k_cpu.h"
#include "../k_defs.h"


uint32_t k_mem_MapLinearAddress(uintptr_t linear_address, uintptr_t physical_address, uint32_t flags)
{
    if(!linear_address || linear_address >= (uintptr_t)K_MEM_PMAP_PENTRY_PAGE_POINTER(0))
    {
        return K_STATUS_FORBIDDEN;
    }

    uint32_t pdir_index = K_MEM_PDIR_INDEX(linear_address);
    uint32_t ptable_index = K_MEM_PTABLE_INDEX(linear_address);
    struct k_mem_pentry_t *pdir = K_MEM_PMAP_PDIR_PAGE_POINTER;
    struct k_mem_pentry_t *pentry = pdir + pdir_index;

    flags |= K_MEM_PENTRY_FLAG_USED | K_MEM_PENTRY_FLAG_READ_WRITE | K_MEM_PENTRY_FLAG_PRESENT;

    if(!(pentry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE))
        {
            /* pdir entry not used and we're mapping a small page. For that we'll need to allocate a 
            new page table */
            pentry->entry = k_mem_AllocPhysicalPage(0) | flags;

            if(!pentry->entry)
            {
                /* maybe should try to wait for the memory manager to free up some physical memory...? */
                return K_STATUS_OUT_OF_PHYSICAL_MEM;
            }

            /* we'll also need to map the page that contains this new page table in the pmap page table, 
            so we can modify its entries */
            struct k_mem_pentry_t *ptable = K_MEM_PMAP_PTABLE_PAGE_POINTER + pdir_index;
            // k_cpu_InvalidateTLB((uint32_t)ptable);

            ptable->entry = pentry->entry;
            ptable = K_MEM_PMAP_PENTRY_PAGE_POINTER(pdir_index);

            for(uint32_t entry_index = 0; entry_index < 1024; entry_index++)
            {
                ptable[entry_index].entry = 0;
                // k_cpu_InvalidateTLB(&ptable[entry_index]);
            }
            
        }
    }

    if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
    {
        return K_STATUS_LINEAR_ADDRESS_IN_USE;
    }

    if(!(flags & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        pentry = K_MEM_PMAP_PENTRY_PAGE_POINTER(pdir_index) + ptable_index;

        if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            // k_printf("well, shit... %x\n", linear_address);
            return K_STATUS_LINEAR_ADDRESS_IN_USE;
        }
    }

    pentry->entry = physical_address | flags;
    k_cpu_InvalidateTLB((uint32_t)linear_address);

    return K_STATUS_OK;
}

uint32_t k_mem_UnmapLinearAddress(uintptr_t linear_address)
{
    if(!linear_address || linear_address >= (uintptr_t)K_MEM_PMAP_PENTRY_PAGE_POINTER(0))
    {
        return K_STATUS_FORBIDDEN;
    }

    uint32_t pdir_index = K_MEM_PDIR_INDEX(linear_address);
    uint32_t ptable_index = K_MEM_PTABLE_INDEX(linear_address);
    struct k_mem_pentry_t *pdir = K_MEM_PMAP_PDIR_PAGE_POINTER;
    struct k_mem_pentry_t *pentry = pdir + pdir_index;

    if(!(pentry->entry & K_MEM_PENTRY_FLAG_USED))
    {
        return K_STATUS_LINEAR_ADDRESS_NOT_IN_USE;
    }

    if(!(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE))
    {
        pentry = K_MEM_PMAP_PENTRY_PAGE_POINTER(pdir_index) + ptable_index;

        if(!(pentry->entry & K_MEM_PENTRY_FLAG_USED))
        {
            return K_STATUS_LINEAR_ADDRESS_NOT_IN_USE;
        }
    }

    pentry->entry = 0;
    k_cpu_InvalidateTLB(linear_address);

    return K_STATUS_OK;
}

uint32_t k_mem_IsLinearAddressMapped(uintptr_t linear_address)
{
    if(!linear_address || linear_address >= (uintptr_t)K_MEM_PMAP_PENTRY_PAGE_POINTER(0))
    {
        return K_STATUS_FORBIDDEN;
    }

    uint32_t pdir_index = K_MEM_PDIR_INDEX(linear_address);
    uint32_t ptable_index = K_MEM_PTABLE_INDEX(linear_address);
    struct k_mem_pentry_t *pdir = K_MEM_PMAP_PDIR_PAGE_POINTER;
    struct k_mem_pentry_t *pentry = pdir + pdir_index;

    if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
    {
        if(pentry->entry & K_MEM_PENTRY_FLAG_BIG_PAGE)
        {
            return K_STATUS_LINEAR_ADDRESS_IN_USE;
        }

        pentry = K_MEM_PMAP_PENTRY_PAGE_POINTER(pdir_index) + ptable_index;

        if(pentry->entry & K_MEM_PENTRY_FLAG_USED)
        {
            return K_STATUS_LINEAR_ADDRESS_IN_USE;
        }
    }

    return K_STATUS_LINEAR_ADDRESS_NOT_IN_USE;
}