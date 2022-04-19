#include "pup.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../dsk/dsk.h"
#include "../sys/term.h"
#include "../mem/pmap.h"
#include "../cpu/k_cpu.h"

// uint32_t k_fs_pup_block_size;
// uint8_t *k_fs_pup_disk_buffer;
// struct k_fs_pup_root_t *k_fs_pup_root;

void k_fs_PupMountVolume(struct k_fs_vol_t *volume)
{
    volume->data = k_rt_Malloc(sizeof(struct k_fs_pup_volume_t), 4);
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    uint32_t root_address = volume->partition.start * volume->partition.disk->block_size;
    k_dsk_Read(volume->partition.disk, root_address, sizeof(struct k_fs_pup_root_t), &pup_volume->root);
    // pup_volume->cached_blocks_base = k_rt_Malloc(pup_volume->block_size * K_FS_PUP_BLOCKS_PER_SLOT * K_FS_PUP_CACHE_SLOT_COUNT, 4);
    
    pup_volume->lru_bitmask = 0;
    for(uint32_t set_index = 0; set_index < K_FS_PUP_CACHE_SET_COUNT; set_index++)
    {
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        set->first_entry = NULL;
        set->last_entry = NULL;
        set->read_count = 0;
        set->write_lock = 0;
        // slot->buffer = pup_volume->cached_blocks_base + slot_index * pup_volume->block_size * K_FS_PUP_BLOCKS_PER_SLOT;
        
        // slot->valid_blocks = 0;
        // slot->access_age = 0;
        // slot->flags = 0;
        // slot->first_block = 0;
    }
    
    k_sys_TerminalPrintf("pup volume mounted, block size: %d bytes\n", pup_volume->root.block_size);
    
    // pup_volume->used_slots = 0;
}

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume)
{
    (void)volume;
}

uint32_t k_fs_PupTryCopyEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry, uint8_t *block_buffer, uint32_t block_start, uint32_t block_count)
{
    // uint32_t entry_end = entry->first_block + K_FS_PUP_BLOCKS_PER_ENTRY;
    // uint32_t block_buffer_offset = 0;
    // uint32_t entry_buffer_offset = 0;
    // uint32_t copy_size = 0;
    // 
    // if(entry->first_block >= block_start && entry->first_block < block_end)
    // {
    //     block_buffer_offset = entry->first_block - block_start;
    // 
    //     if(entry_end >= block_end)
    //     {
    //         copy_size = block_end - entry->first_block;
    //     }
    //     else
    //     {
    //         copy_size = entry_end - entry->first_block;
    //     }
    // }
    // else if(block_start >= entry->first_block && block_start < entry_end)
    // {
    //     entry_buffer_offset = block_start - entry->first_block;
    // 
    //     if(block_end >= entry_end)
    //     {
    //         copy_size = entry_end - block_start;
    //     }
    //     else
    //     {
    //         copy_size = block_end - block_start;
    //     }
    // }
    // else
    // {
    //     return 0;
    // }
    // 
    // k_fs_PupTouchEntry(volume, set_index, entry);
    // 
    // block_buffer_offset *= volume->root.block_size;
    // entry_buffer_offset *= volume->root.block_size;
    // copy_size *= volume->block_size;
    // k_rt_CopyBytes(block_buffer + block_buffer_offset, entry->buffer + entry_buffer_offset, copy_size * volume->block_size);
    // return copy_size;
}

void k_fs_PupTouchEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry)
{
    struct k_fs_pup_cset_t *set = volume->cache_sets + set_index;
    
    k_rt_SpinLock(&set->write_lock);
    
    if(entry->prev)
    {
        entry->prev->next = entry->next;
        
        if(entry->next)
        {
            entry->next->prev = entry->prev;
        }
        else
        {
            set->last_entry = set->last_entry->prev;
        }
        
        entry->next = set->first_entry;
        set->first_entry = entry;
    }
    
    k_rt_SpinUnlock(&set->write_lock);
    
    // do
    // {
    //     uint32_t start_lru_bitmask = volume->lru_bitmask;
    //     uint32_t cur_lru_bitmask = start_lru_bitmask;
    //     uint32_t cur_divisor = K_FS_PUP_CACHE_SET_COUNT / 2;
    // 
    //     for(uint32_t index = 1; index <= 16; index <<= 1)
    //     {
    //         uint32_t split_bit = (1 << (set_index / cur_divisor));
    //         cur_lru_bitmask ^= split_bit;
    //         cur_divisor >>= 1;
    // 
    // 
    // 
    //         // uint32_t test_mask = (1 << index) - 1;
    // 
    //         // uint32_t offset_multiplier = shifted_access_bitmask & test_mask;
    //         // set_index += set_index_offset * offset_multiplier;
    //         // set_index_offset >>= 1;
    //         // shifted_access_bitmask >>= index;
    //     }
    // }
    // while(!)
}

void k_fs_PupRead(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    uint32_t copied_blocks = 0;
    uint32_t block_end = block_start + block_count;
    uint8_t *block_buffer = (uint8_t *)buffer;
    uint32_t request_ranges_count = 0;
    struct k_fs_pup_range_t request_ranges[4];
    
    // k_sys_TerminalPrintf("read %x blocks, starting at %x\n", block_count, block_start);
    
    while(copied_blocks < block_count)
    {
        // uint32_t set_index = K_FS_PUP_CACHE_SET_INDEX(block_start);
        uint32_t set_index = 0;
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        
        k_rt_Inc32Wrap(&set->read_count);
        k_rt_SpinWait(&set->write_lock);
        
        struct k_fs_pup_centry_t *entry = set->first_entry;
        uint32_t block_buffer_offset = 0;
        uint32_t entry_buffer_offset = 0;
        uint32_t copy_size = 0;
        
        while(entry)
        {
            struct k_fs_pup_centry_t *next_entry = entry->next;
            uint32_t entry_end = entry->first_block + K_FS_PUP_BLOCKS_PER_ENTRY;
            
            if(entry->first_block >= block_start && entry->first_block < block_end)
            {
                block_buffer_offset = entry->first_block - block_start;
        
                if(entry_end >= block_end)
                {
                    copy_size = block_end - entry->first_block;
                }
                else
                {
                    copy_size = entry_end - entry->first_block;
                }
            }
            else if(block_start >= entry->first_block && block_start < entry_end)
            {
                entry_buffer_offset = block_start - entry->first_block;
        
                if(block_end >= entry_end)
                {
                    copy_size = entry_end - block_start;
                }
                else
                {
                    copy_size = block_end - block_start;
                }
            }
            else
            {
                entry = next_entry;
                continue;
            }
            
            /* found an entry, so touch it to keep mark it as recently accessed */
            k_fs_PupTouchEntry(volume, set_index, entry);
            break;
        }
        
        k_rt_Dec32Wrap(&set->read_count);
        
        if(!entry)
        {
            // k_sys_TerminalPrintf("Entry for block %x not in cache...\n", block_start);
            /* entry not in cache, so load it from disk */
            uint32_t block_multiplier = pup_volume->root.block_size / volume->partition.disk->block_size;
            uint32_t read_count = K_FS_PUP_BLOCKS_PER_ENTRY * pup_volume->root.block_size;
            uint32_t read_start = (block_start & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1))) * pup_volume->root.block_size;
            read_start += volume->partition.start * volume->partition.disk->block_size;
            entry = k_fs_PupAllocCacheEntry(volume);
            k_dsk_Read(volume->partition.disk, read_start, read_count, entry->buffer); 
            entry_buffer_offset = block_start % pup_volume->root.block_size;
            copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - entry_buffer_offset;
            block_buffer_offset = block_start;
            k_fs_PupCacheEntry(pup_volume, entry);
            // k_sys_TerminalPrintf("entry %x for block %x cached\n", entry, block_start);
        }
        // else
        // {
        //     // k_sys_TerminalPrintf("cached entry %x for block %x found\n", entry, block_start);
        // }
        
        if(copy_size > block_count)
        {
            copy_size = block_count;
        }
        
        block_buffer_offset *= pup_volume->root.block_size;
        entry_buffer_offset *= pup_volume->root.block_size;
        k_rt_CopyBytes(block_buffer + block_buffer_offset, entry->buffer + entry_buffer_offset, copy_size * pup_volume->root.block_size);
        copied_blocks += copy_size;
        block_start += copy_size;
    }
}

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    
}

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_vol_t *volume)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_centry_t *entry = NULL;
    // struct k_fs_pup_centry_t *entry = pup_volume->entry_pool;
    
    // while(1)
    // {
// struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
    // while(entry && !k_rt_CmpXcgh((uintptr_t *)pool_head, (uintptr_t)entry, (uintptr_t)entry->next, (uintptr_t *)&entry));
    
    // if(!entry)
    do
    {
        /* no entry in the pool */
        
        if(pup_volume->allocated_memory < K_FS_PUP_MAX_CACHE_MEM)
        {
            /* we're still under the memory budget for this disk, so malloc a new cache entry */
            uint32_t alloc_size = sizeof(struct k_fs_pup_centry_t) + pup_volume->root.block_size * K_FS_PUP_BLOCKS_PER_ENTRY;
            /* FIXME: race condition when modifying allocated_memory */
            pup_volume->allocated_memory += alloc_size;
            // k_sys_TerminalPrintf("alloc %d bytes\n", alloc_size);
            entry = k_rt_Malloc(alloc_size, 4);
        }
        else
        {
            /* well, crap. We're over budget here, so keep evicting the oldest entry 
            in the cache and tring until we get one */
            entry = k_fs_PupEvictOldestEntry(volume);
        }
    }
    while(!entry);
    // }
    
    return entry;
}

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    (void)volume;
    (void)entry;
    // struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    // entry->next = pup_volume->entry_pool;
    // while(!k_rt_CmpXcgh((uintptr_t *)&pup_volume->entry_pool, (uintptr_t)entry->next, (uintptr_t)entry, (uintptr_t *)&entry->next));
}

void k_fs_PupCacheEntry(struct k_fs_pup_volume_t *volume, struct k_fs_pup_centry_t *entry)
{
    /* FIXME: there's a race condition between the time a thread requests a line from disk
    and when it get inserted in the cache. If another thread tries to read the same location
    it won't find the line in cache and will also request it. Insertion in the cache should
    check for duplicated entries */
    
    struct k_fs_pup_cset_t *set = volume->cache_sets;
    /* wait until no one is touching this line */
    while(set->read_count);
    
    k_rt_SpinLock(&set->write_lock);
    if(!set->first_entry)
    {
        set->first_entry = entry;
    }
    else
    {
        set->last_entry->next = entry;
        entry->prev = set->last_entry;
    }
    set->last_entry = entry;
    k_rt_SpinUnlock(&set->write_lock);
}

struct k_fs_pup_centry_t *k_fs_PupEvictOldestEntry(struct k_fs_vol_t *volume)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    uint32_t start_access_bitmask = pup_volume->lru_bitmask;
    uint32_t shifted_access_bitmask = start_access_bitmask;
    uint32_t set_index_offset = K_FS_PUP_CACHE_SET_COUNT / 2;
    uint32_t set_index = 0;
    
    for(uint32_t index = 1; index <= 16; index <<= 1)
    {
        uint32_t test_mask = (1 << index) - 1;
        uint32_t offset_multiplier = shifted_access_bitmask & test_mask;
        set_index += set_index_offset * offset_multiplier;
        set_index_offset >>= 1;
        shifted_access_bitmask >>= index;
    }
    
    struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
    
    // k_rt_SpinWait(&set->read_lock);
    while(set->read_count);
    
    k_rt_SpinLock(&set->write_lock);
    struct k_fs_pup_centry_t *oldest_entry = set->last_entry;
    set->last_entry = oldest_entry->prev;
    set->last_entry->next = NULL;
    k_rt_SpinUnlock(&set->write_lock);
    
    return oldest_entry;
    // k_fs_PupFreeCacheEntry(oldest_entry);
}

// struct k_fs_pup_bcache_t *k_fs_PupFindBlock(struct k_fS_vol_t *volume, uint32_t block_address)
// {
//     struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
//     struct k_fs_pup_bcache_t *cached_block = NULL;
//     for(uint32_t block_index = 0; block_index < pup_volume->cached_count; block_index++)
//     {
//         struct k_fs_pup_bcache_t *block = pup_volume->cached_blocks + block_index;
//         if(block.range.first_block <= block_address && block_address <= block->range.first_block + block->range.block_count)
//         {
//             cached_block = block;
//             block->access_age = 0;
//         }
// 
//         if(block->access_age)
//         {
//             block->access_age--;
//         }
//     }
// 
//     return cached_block;
// }

void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
{
    
}

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
    
}