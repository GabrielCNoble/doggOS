#include "pup.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../rt/string.h"
#include "../dsk/dsk.h"
#include "../sys/term.h"
#include "../sys/defs.h"
#include "../mem/pmap.h"
#include "../cpu/k_cpu.h"
#include "fs.h"

// uint32_t k_fs_pup_block_size;
// uint8_t *k_fs_pup_disk_buffer;
// struct k_fs_pup_root_t *k_fs_pup_root;

void k_fs_PupMountVolume(struct k_fs_vol_t *volume)
{
    volume->data = k_rt_Malloc(sizeof(struct k_fs_pup_vol_t), 4);
    // volume->data = k_rt_Malloc(1024, 4);
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    k_fs_ReadVolumeBytes(volume, volume->partition.disk->block_size, 0, 0, sizeof(struct k_fs_pup_root_t), &pup_volume->root);
    // uint32_t root_address = volume->partition.first_block * volume->partition.disk->block_size;
    // k_sys_TerminalPrintf("partition: %d %d\n", volume->partition.first_block, volume->partition.disk->block_size);
    // k_sys_TerminalPrintf("root size: %x\n", (uint32_t)sizeof(struct k_fs_pup_root_t));
    // k_dsk_Read(volume->partition.disk, root_address, sizeof(struct k_fs_pup_root_t), &pup_volume->root);

    // for(uint32_t index = 0; index < K_FS_PUP_IDENT; index++)
    // {
    //     k_sys_TerminalPrintf("%c", pup_volume->root.ident[index]);
    // }

    uint32_t bitmask_block_count = pup_volume->root.alloc_start - pup_volume->root.bitmask_start;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    pup_volume->lru_bitmask = 0;
    pup_volume->entry_pool = NULL;
    pup_volume->bitmask_blocks = k_rt_Malloc(bitmask_block_count * block_size, 4);

    k_fs_ReadVolumeBlocks(volume, block_size, pup_volume->root.bitmask_start, bitmask_block_count, pup_volume->bitmask_blocks);
    // k_sys_TerminalPrintf("first alloc byte: %x\n", pup_volume->bitmask_blocks[0]);
    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();
    
    for(uint32_t set_index = 0; set_index < K_FS_PUP_CACHE_SET_COUNT; set_index++)
    {
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        set->first_entry = NULL;
        set->last_entry = NULL;
        set->read_count = 0;
        set->write_lock = 0;
    }
}

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume)
{
    if(volume && volume->data)
    {
        /* FIXME: THIS IS NOT, AT ALL, HOW YOU UNMOUNT A VOLUME!!!! */
        k_rt_Free(volume->data);
        volume->data = NULL;
    }
}

uint32_t k_fs_PupFormatVolume(struct k_fs_vol_t *volume, void *args)
{
    struct k_fs_pup_format_args_t *format_args = (struct k_fs_pup_format_args_t *)args;
    struct k_fs_pup_root_t root = {};

    uint32_t nodes_per_block = format_args->block_size / sizeof(struct k_fs_pup_node_t);
    uint32_t mask = 0x80000000;
    root.node_index_bits = 32;

    /* compute how many bits we need to store a node index */
    while(mask)
    {
        if(nodes_per_block & mask)
        {
            break;
        }

        mask >>= 1;
        root.node_index_bits--;
    }

    root.block_size_shift = 31;
    mask = 0x80000000;
    /* compute block size shift */
    while(mask)
    {
        if(format_args->block_size & mask)
        {
            break;
        }

        mask >>= 1;
        root.block_size_shift--;
    }

    k_rt_StrCpy(root.ident, sizeof(root.ident), K_FS_PUP_MAGIC);
    root.bitmask_start = 1;
    /* each byte in a bitmask block contains state for 4 blocks, and we need
    1 block to contain the bitmask block. */
    uint32_t bitmask_block_count = format_args->block_count / (format_args->block_size * 4 + 1);
    if(!bitmask_block_count)
    {
        bitmask_block_count++;
    }

    root.alloc_start = root.bitmask_start + bitmask_block_count;
    root.block_count = format_args->block_count;

    /* clear root block */
    k_fs_ClearVolumeBlocks(volume, format_args->block_size, 0, 1);
    /* clear bitmask blocks */
    k_fs_ClearVolumeBlocks(volume, format_args->block_size, root.bitmask_start, bitmask_block_count);

    /* allocate the first available block for nodes, and put the root node at the very beginning */
    uint8_t first_node_block = K_FS_PUP_BLOCK_STATUS_NODE;
    k_fs_WriteVolumeBytes(volume, format_args->block_size, root.bitmask_start, 0, sizeof(first_node_block), &first_node_block);
    root.root_node = K_FS_PUP_MAKE_LINK(root.alloc_start, 0, root.node_index_bits);
    // k_sys_TerminalPrintf("node index bits: %d\n", (uint32_t)root.node_index_bits);
    /* write file system root */
    k_fs_WriteVolumeBytes(volume, format_args->block_size, 0, 0, sizeof(struct k_fs_pup_root_t), &root);

    struct k_fs_pup_node_t root_node = {
        .type = K_FS_PUP_NODE_TYPE_DIR,
        .parent = root.root_node,   
    };
    /* initialize root node */
    k_fs_WriteVolumeBytes(volume, format_args->block_size, root.alloc_start, 0, sizeof(struct k_fs_pup_node_t), &root_node);

    return 0;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupTouchEntry(struct k_fs_pup_vol_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry)
{
    if(entry)
    {
        struct k_fs_pup_cset_t *set = volume->cache_sets + set_index;
        return;
        
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
    }
}

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_pup_vol_t *volume)
{
    // struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_centry_t *entry = NULL;
    uint32_t block_size = 1 << volume->root.block_size_shift;
    
    while(!entry)
    {
        // struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
        // entry = pup_volume->entry_pool;
        // 
        // while(entry && !k_rt_CmpXchg((uintptr_t *)pool_head, (uintptr_t)entry, (uintptr_t)entry->next, (uintptr_t *)&entry));
        
        if(!entry)
        {
            // k_sys_TerminalPrintf("%x\n", volume);
            // k_cpu_DisableInterrupts();
            // k_cpu_Halt();
            /* no entry in the pool */        
            if(volume->allocated_memory < K_FS_PUP_MAX_CACHE_MEM)
            {
                
                /* we're still under the memory budget for this disk, so malloc a new cache entry */
                uint32_t alloc_size = sizeof(struct k_fs_pup_centry_t) + block_size * (K_FS_PUP_BLOCKS_PER_ENTRY);
                // k_sys_TerminalPrintf("entry size: %d\n", alloc_size);
                /* FIXME: race condition when modifying allocated_memory */
                volume->allocated_memory += alloc_size;
                entry = k_rt_Malloc(alloc_size, 4);
            }
            else
            {
                /* well, crap. We're over budget here, so keep evicting the oldest entry 
                in the cache and tring until we get one */
                entry = k_fs_PupDropOldestEntry(volume);
            }
            
            // k_sys_TerminalPrintf("blah\n");
        }
    }
    
    entry->next = NULL;
    entry->prev = NULL;
    entry->ref_count = 0;
    
    return entry;
}

// struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntryForAddress(struct k_fs_vol_t *volume, uint32_t address)
// {
//     struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
//     /* FIXME: compute set from address */
//     struct k_fs_pup_cset_t *set = pup_volume->cache_sets;

//     struct k_fs_pup_centry_t *entry = k_fs_PupAllocCacheEntry(volume);
//     entry->flags = K_FS_PUP_CENTRY_FLAG_LOAD_PENDING;
//     entry->first_block = address & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));
    
//     k_rt_SpinLock(&set->write_lock);

//     k_rt_SpinUnlock(&set->write_lock);
// }

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
    
    // entry->next = pup_volume->entry_pool;
    
    // while(!k_rt_CmpXchg((uintptr_t *)pool_head, (uintptr_t)entry->next, (uintptr_t)entry, (uintptr_t *)&entry->next))
    // {
    //     /* nothing to see here */
    // }

    if(entry)
    {
        k_rt_Free(entry);
    }
}

void k_fs_PupCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry)
{    
    if(!set)
    {
        set = &volume->cache_sets[0];
    }
    
    /* wait until no one is touching this line */
    // k_sys_TerminalPrintf("k_fs_PupCacheEntry: entry %x, cache %x\n", entry, set);
    while(set->read_count);
    // k_sys_TerminalPrintf("k_fs_PupCacheEntry: read count is zero\n");
    k_cpu_MemFence();
    k_rt_SpinLock(&set->write_lock);
    // k_sys_TerminalPrintf("k_fs_PupCacheEntry: write lock acquired\n");
    set->cur_insert = entry;

    // struct k_fs_pup_centry_t *test_entry = set->first_entry;
    // while(test_entry)
    // {
    //     // k_sys_TerminalPrintf("k_fs_PupCacheEntry: %x\n", test_entry);
    //     test_entry = test_entry->next;
    // }

    if(!set->first_entry)
    {
        set->first_entry = entry;
        // set->last_entry = entry;
    }
    else
    {
        struct k_fs_pup_centry_t *duplicate = set->first_entry;

        while(duplicate)
        {
            if(duplicate->first_block == entry->first_block)
            {
                /* entry already in cache */
                break;
            }

            duplicate = duplicate->next;
        }

        if(!duplicate)
        {
            set->last_entry->next = entry;
            entry->prev = set->last_entry;   
        }
    }
    set->last_entry = entry;
    set->cur_insert = NULL;
    k_rt_SpinUnlock(&set->write_lock);
}

/*
================
k_fs_PupFindEntryInSetWithCopyFields

searches a cache set for an entry that overlaps with the [block_start - block_end] interval, 
and optionally fills [copy] with the proper values considering how the interval overlaps the entry

================
*/
struct k_fs_pup_centry_t *k_fs_PupFindEntryInSetWithCopyFields(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end, struct k_fs_pup_centry_copy_t *copy)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

    if(!set)
    {
        /* FIXME: compute the proper cache set for the requested block address */
        set = &pup_volume->cache_sets[0];
    }

    struct k_fs_pup_centry_t *entry = set->first_entry;
    // k_sys_TerminalPrintf("k_fs_PupFindEntryInSetWithCopyFields: %x, %x\n", block_start, block_end);
    // uint32_t block_buffer_offset = 0;
    uint32_t entry_buffer_offset = 0;
    uint32_t copy_size = 0;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    
    k_rt_Inc32Wrap(&set->read_count);
    k_cpu_MemFence();
    /* wait until whoever might be writing to this set is done */
    k_rt_SpinWait(&set->write_lock);

    while(entry)
    {
        uint32_t entry_end = entry->first_block + K_FS_PUP_BLOCKS_PER_ENTRY;
        // k_sys_TerminalPrintf("test entry %x (%x - %x)\n", entry, entry->first_block, entry_end);
        if(entry->first_block >= block_start && entry->first_block < block_end)
        {
            // block_buffer_offset = entry->first_block - block_start;
    
            if(entry_end >= block_end)
            {
                copy_size = block_end - entry->first_block;
            }
            else
            {
                copy_size = entry_end - entry->first_block;
            }

            // k_sys_TerminalPrintf("entry %s is usable\n");
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
            entry = entry->next;
            continue;
        }
        
        // k_sys_TerminalPrintf("entry %x is usable\n", entry);
        if(copy)
        {
            copy->entry_buffer_offset = entry_buffer_offset;
            // copy->data_buffer_offset = block_buffer_offset;
            copy->copy_size = copy_size;

            // k_sys_TerminalPrintf("copy fields for entry %x\n", entry);
        }

        /* to make sure this entry doesn't get evicted until everyone is done with it */
        k_rt_Inc32Wrap(&entry->ref_count);
        
        break;
    }

    // k_sys_TerminalPrintf("k_fs_PupFindEntryInSetWithCopyFields: entry %x\n", entry);

    /* give threads wanting to write to the cache a go ahead */ 
    k_rt_Dec32Wrap(&set->read_count);

    // if(!entry)
    // {
    //     /* entry not in cache, so load it from disk */
    //     // k_sys_TerminalPrintf("k_fs_PupFindEntryInSetWithCopyFields: no entry for block %x\n", block_start);
    //     /* align block_start to a set boundary */
    //     uint32_t first_block = block_start & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));
    //     entry = k_fs_PupAllocCacheEntry(pup_volume);
    //     entry->first_block = first_block;
        
    //     if(copy)
    //     {
    //         copy->entry_buffer_offset = block_start % K_FS_PUP_BLOCKS_PER_ENTRY;
    //         // copy.data_buffer_offset = buffer_cursor;
    //         copy->copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - copy->entry_buffer_offset;
    //     }
        
    //     k_fs_ReadVolumeBlocks(volume, block_size, first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
    //     k_fs_PupCacheEntry(pup_volume, set, entry);
    // }
    
    // k_sys_TerminalPrintf("entry %x\n", entry);
    
    return entry;
}

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSet(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end)
{
    return k_fs_PupFindEntryInSetWithCopyFields(volume, set, block_start, block_end, NULL);
}

void k_fs_PupReleaseEntry(struct k_fs_pup_centry_t *entry)
{
    if(entry && entry->ref_count)
    {
        k_rt_Dec32Wrap(&entry->ref_count);
    }
}

// void k_fs_ReleaseEntry(struct )

struct k_fs_pup_centry_t *k_fs_PupDropOldestEntry(struct k_fs_pup_vol_t *volume)
{
    // struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    // uint32_t start_access_bitmask = pup_volume->lru_bitmask;
    // uint32_t shifted_access_bitmask = start_access_bitmask;
    // uint32_t set_index_offset = K_FS_PUP_CACHE_SET_COUNT / 2;
    // uint32_t set_index = 0;
    // 
    // for(uint32_t index = 1; index <= 16; index <<= 1)
    // {
    //     uint32_t test_mask = (1 << index) - 1;
    //     uint32_t offset_multiplier = shifted_access_bitmask & test_mask;
    //     set_index += set_index_offset * offset_multiplier;
    //     set_index_offset >>= 1;
    //     shifted_access_bitmask >>= index;
    // }
    // 
    // struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
    // 
    // while(set->read_count);
    // 
    // k_rt_SpinLock(&set->write_lock);
    // struct k_fs_pup_centry_t *oldest_entry = set->last_entry;
    // set->last_entry = oldest_entry->prev;
    // set->last_entry->next = NULL;
    // k_rt_SpinUnlock(&set->write_lock);
    // 
    // return oldest_entry;
    
    return NULL;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupRead(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_end = block_start + block_count;
    uint8_t *block_buffer = (uint8_t *)buffer;
    uint32_t buffer_cursor = 0;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    // uint32_t request_ranges_count = 0;
    // struct k_fs_pup_range_t request_ranges[4];

    // k_sys_TerminalPrintf("k_fs_PupRead: %x, %x\n", block_start, block_count);
    
    while(block_start < block_end)
    {
        
        uint32_t set_index = 0;
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        
        struct k_fs_pup_centry_copy_t copy = {};
        struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSetWithCopyFields(volume, set, block_start, block_end, &copy);
        
        if(!entry)
        {
            /* entry not in cache, so load it from disk */


            /* align block_start to a set boundary */
            uint32_t first_block = block_start & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));
            entry = k_fs_PupAllocCacheEntry(pup_volume);
            entry->first_block = first_block;
            entry->ref_count = 1;
            
            copy.entry_buffer_offset = block_start % K_FS_PUP_BLOCKS_PER_ENTRY;
            copy.copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - copy.entry_buffer_offset;
            
            k_fs_ReadVolumeBlocks(volume, block_size, first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
            k_fs_PupCacheEntry(pup_volume, set, entry);
        }

        k_fs_PupTouchEntry(pup_volume, set_index, entry);
        
        if(copy.copy_size > block_count)
        {
            copy.copy_size = block_count;
        }

        // k_sys_TerminalPrintf("k_fs_PupRead: %d %d\n", copy.entry_buffer_offset, copy.copy_size);
        
        // copy.data_buffer_offset *= pup_volume->root.block_size;
        copy.entry_buffer_offset *= block_size;

        k_rt_CopyBytes(block_buffer + buffer_cursor, entry->buffer + copy.entry_buffer_offset, copy.copy_size * block_size);
        k_fs_PupReleaseEntry(entry);
        
        block_start += copy.copy_size;
        buffer_cursor += copy.copy_size * block_size;
        block_count -= copy.copy_size;
    }
    
}

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_end = block_start + block_count;
    uint8_t *block_buffer = (uint8_t *)buffer;
    uint32_t buffer_cursor = 0;
    
    while(block_start < block_end)
    {
        // uint32_t set_index = 0;
        // struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        

        // k_rt_Inc32Wrap(&set->read_count);
        // /* should there be a memory fence here? */
        // k_rt_SpinWait(&set->write_lock);
        
        // struct k_fs_pup_centry_copy_t copy = {};
        // struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSetWithCopyFields(set, block_start, block_end, &copy);
        
        // if(!entry)
        // {
        //     /* entry not in cache, so load it from disk */

        //     /* align block_start to a set boundary */
        //     // uint32_t first_block = block_start & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));
        //     // uint32_t block_count = K_FS_PUP_BLOCKS_PER_ENTRY;
        //     // entry = k_fs_PupAllocCacheEntry(volume);
        //     // entry->first_block = first_block;
            
        //     // copy.entry_buffer_offset = block_start % K_FS_PUP_BLOCKS_PER_ENTRY;
        //     // copy.data_buffer_offset = buffer_cursor;
        //     // copy.copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - copy.entry_buffer_offset;
            
        //     // k_fs_ReadVolume(volume, pup_volume->root.block_size, first_block, block_count, entry->buffer);
        //     // k_fs_PupCacheEntry(pup_volume, set, entry);


        // }
        
        // if(copy.copy_size > block_count)
        // {
        //     copy.copy_size = block_count;
        // }
        
        // copy.data_buffer_offset *= pup_volume->root.block_size;
        // copy.entry_buffer_offset *= pup_volume->root.block_size;

        // k_rt_CopyBytes(block_buffer + copy.data_buffer_offset, entry->buffer + copy.entry_buffer_offset, copy.copy_size * pup_volume->root.block_size);

        // k_fs_PupTouchEntry(pup_volume, set_index, entry);
        // /* should there be a memory fence here? */
        // k_rt_Dec32Wrap(&set->read_count);
        
        // block_start += copy.copy_size;
        // buffer_cursor += copy.copy_size;
        // block_count -= copy.copy_size;
    }
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

// void k_fs_PupNextPathComponent(struct k_fs_pup_path_t *path)
// {
//     // if(path && path->path && path->path[path->end])
//     // {
//     //     if(!path->start)
//     //     {
//     //         /* white spaces are only meaningless at the start of a path */
//     //         while(path->path[path->start] == ' ' && path->path[path->start] != '\0')
//     //         {
//     //             path->start++;
//     //         }
//     //     }
//     //     else
//     //     {
//     //         path->start = path->end;
//     //     }
//     // 
//     //     if(path->path[path->start] == '/')
//     //     {
//     //         path->start++;
//     // 
//     //         while(path->path[path->start] == '/' && path->path[path->start] != '\0')
//     //         {
//     //             path->start++;
//     //         }
//     //     }
//     // 
//     //     path->end = path->start;
//     // 
//     //     while(path->path[path->end] != '/' && path->path[path->end] != '\0')
//     //     {
//     //         path->end++;
//     //     }
//     // }
// }
/*
================
k_fs_PupGetBlock

gets a block based on its address and caches in the provided cache set. The set can come either from 
the file system cache, or can be a local variable. When it's a local variable, it serves to keep an
entry alive while code is accessing it and while avoiding touching the file system cache for each
access.
================
*/
struct k_fs_pup_centry_t *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint64_t block_address)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSet(volume, set, (uint32_t)block_address, (uint32_t)block_address + 1);

    if(!entry)
    {
        entry = k_fs_PupAllocCacheEntry(pup_volume);
        entry->first_block = (uint32_t)block_address;    
        entry->first_block &= ~(K_FS_PUP_BLOCKS_PER_ENTRY - 1);
        k_fs_PupRead(volume, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
        k_fs_PupCacheEntry(pup_volume, set, entry);
    }
    
    return entry;
}

struct k_fs_pup_range_t k_fs_PupAllocBlocks(struct k_fs_vol_t *volume, uint32_t count, uint32_t alloc_type)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_range_t range = {};
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    uint32_t block_count = pup_volume->root.block_count - pup_volume->root.alloc_start;
    uint32_t test_status = K_FS_PUP_BLOCK_STATUS_FREE;
    uint32_t range_start = 0;
    uint32_t range_count = 0;

    /* TODO: refactor all of this to something less dumb. */
    switch(alloc_type)
    {
        case K_FS_PUP_BLOCK_STATUS_DATA:
            test_status = K_FS_PUP_BLOCK_STATUS_FREE;
        break;

        case K_FS_PUP_BLOCK_STATUS_NODE:
            test_status = K_FS_PUP_BLOCK_STATUS_NODE;
        break;
    }

    /* TODO: refactor this loop to something a bit more efficient. Adding a second
    loop inside, that iterates over a single byte will allow computing some stuff
    once for the iterations of this inner loop.
    
    It may also be possible to test a whole byte at once. This will be testing the
    status for four consecutive blocks at once. */
    for(uint32_t block_index = 0; block_index < block_count; block_index++)
    {   
        /* 4 blocks per byte */
        uint8_t bitmask = pup_volume->bitmask_blocks[block_index >> 2];
        uint32_t bitmask_shift = (block_index % 4) << 1;
        uint32_t status = bitmask_shift >> bitmask_shift;

        if(status == test_status || status == K_FS_PUP_BLOCK_STATUS_FREE)
        {
            if(!range_start)
            {
                range_start = block_index;
                range_count = 0;
            }

            range_count++;

            if(range_count == count)
            {
                // range = K_FS_PUP_RANGE_MAKE_RANGE(range_start + pup_volume->root.alloc_start, range_count);
                range.start = range_start + pup_volume->root.alloc_start;
                range.count = range_count;
                break;
            }
        }
        else
        {
            range_start = 0;
        }
    }

    k_fs_PupSetRangeStatus(volume, &range, alloc_type);

    return range;
}

void k_fs_PupSetRangeStatus(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t status)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume;

    // uint32_t first_block = (uint32_t)K_FS_PUP_RANGE_START(range->contents) - pup_volume->root.alloc_start;
    // uint32_t block_count = (uint32_t)K_FS_PUP_RANGE_COUNT(range->contents);

    uint32_t first_block = (uint32_t)range->start - pup_volume->root.alloc_start;
    uint32_t block_count = (uint32_t)range->count;

    for(uint32_t block_index = first_block; block_index < block_count; block_index++)
    {
        /* 4 blocks per byte */
        uint8_t bitmask = pup_volume->bitmask_blocks[block_index >> 2];
        uint32_t bitmask_shift = (block_index % 4) << 1;
        pup_volume->bitmask_blocks[block_index >> 2] |= status << bitmask_shift;
    }
}

void k_fs_PupFreeBlock(struct k_fs_pup_range_t block)
{

}

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node, struct k_fs_pup_cset_t *cache)
{
    // struct k_fs_pup_node_t *cur_node;
    struct k_fs_pup_node_t *node = NULL;
    struct k_fs_pup_link_t node_link = K_FS_PUP_NULL_LINK;
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    /* local cache set, to keep entries alive while we're using them without needing
    synchronization */
    struct k_fs_pup_cset_t local_cache = {};
    uint32_t path_fragment_cursor = 0;
    uint32_t path_cursor = 0;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    char path_fragment[256];
    
    if(!cache)
    {
        cache = &local_cache;
    }
    
    while(path[path_cursor] == ' ' && path[path_cursor] != '\0')
    {
        path_cursor++;
    }

    // k_sys_TerminalPrintf("%s\n", path);
    if(path[path_cursor] == '/' && !start_node.link)
    {
        path_cursor++;
        node = k_fs_PupGetNode(volume, pup_volume->root.root_node, cache);
        start_node = pup_volume->root.root_node;
    }
    // k_sys_TerminalPrintf("k_fs_PupFindNode: start_node: %x\n", (uint32_t)start_node.link);
    if(start_node.link)
    {
        node = k_fs_PupGetNode(volume, start_node, cache);
        node_link = start_node;
            
        while(path[path_cursor] && node)
        {    
            path_fragment_cursor = 0;
            while(path[path_cursor] != '/' && path[path_cursor])
            {
                path_fragment[path_fragment_cursor] = path[path_cursor];
                path_fragment_cursor++;
                path_cursor++;
            }
            
            if(path[path_cursor] == '/')
            {
                path_cursor++;
            }
            
            path_fragment[path_fragment_cursor] = '\0';
            
            struct k_fs_pup_node_t *next_node = NULL;
            struct k_fs_pup_link_t next_node_link = {};

            if(node->type == K_FS_PUP_NODE_TYPE_DIR)
            {
                if(!k_rt_StrCmp(path_fragment, "."))
                {
                    next_node = node;
                    next_node_link = node_link;
                }                
                else if(!k_rt_StrCmp(path_fragment, ".."))
                {
                    next_node_link = node->parent;
                    next_node = k_fs_PupGetNode(volume, node->parent, cache);
                }
                else
                {
                    for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES && !next_node; range_index++)
                    {
                        struct k_fs_pup_range_t *range = node->ranges + range_index;
                        // uint32_t first_block = K_FS_PUP_RANGE_START(range->contents);
                        uint32_t first_block = (uint32_t)range->start;
                        
                        if(!first_block)
                        {
                            continue;
                        }
                        
                        struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, cache, first_block);
                        uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
                        struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * block_size);
                        // k_sys_TerminalPrintf("%d\n", entry_list->used_count);
                        for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
                        {
                            struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index;
                            // k_sys_TerminalPrintf("aa %s\n", entry->name);
                            if(!entry->node.link)
                            {
                                break;
                            }
                            
                            if(!k_rt_StrCmp(path_fragment, entry->name))
                            {
                                // k_sys_TerminalPrintf("%s\n", entry->name);
                                next_node_link = entry->node;
                                next_node = k_fs_PupGetNode(volume, next_node_link, cache);
                                range_index = K_FS_PUP_MAX_RNODE_RANGES;
                                break;
                            }
                        }
                    }
                }
            }
            
            node = next_node;
            node_link = next_node_link;
        }
        
        if(cache == &local_cache)
        {
            struct k_fs_pup_centry_t *entry = cache->first_entry;
            
            while(entry)
            {
                struct k_fs_pup_centry_t *next_entry = entry->next;
                k_fs_PupFreeCacheEntry(volume, entry);
                entry = next_entry;
            }
        }
    }
    
    return node_link;
}

/*
================
k_fs_PupGetNode

gets a node based on its address and caches in the provided cache set. The set can come either from 
the file system cache, or can be a local variable, to speed up sucessive block reads (for example, when traversing
a directory)
================
*/
struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_cset_t *cache)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_node_t *node = NULL;
    uint32_t node_block = (uint32_t)(node_address.link >> pup_volume->root.node_index_bits);
    uint32_t node_index_mask = (1 << pup_volume->root.node_index_bits) - 1;
    uint32_t node_index = ((uint32_t)node_address.link) & node_index_mask;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    
    /* offset relative to the start of the cache entry */
    node_block %= K_FS_PUP_BLOCKS_PER_ENTRY;
    
    struct k_fs_pup_centry_t *entry = k_fs_PupGetBlock(volume, cache, node_block);
    struct k_fs_pup_node_t *node_list = (struct k_fs_pup_node_t *)(entry->buffer + node_block * block_size);
    node = node_list + node_index;
    
    return node;
}

struct k_fs_pup_link_t k_fs_PupCreateNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, start_node, NULL);

    if(node)
    {

    }
}

struct k_fs_pup_link_t k_fs_PupAllocNode(struct k_fs_vol_t *volume, uint32_t type)
{
    struct k_fs_pup_link_t link = K_FS_PUP_NULL_LINK;

    if(type != K_FS_PUP_NODE_TYPE_NONE)
    {
        struct k_fs_pup_range_t node_block = k_fs_PupAllocBlocks(volume, 1, K_FS_PUP_BLOCK_STATUS_NODE);
        struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

        // if(K_FS_PUP_RANGE_COUNT(node_block.contents))
        if(node_block.count)
        {
            // uint32_t block_index = K_FS_PUP_RANGE_START(node_block.contents);
            uint32_t block_index = (uint32_t)node_block.start;
            struct k_fs_pup_centry_t *entry = k_fs_PupGetBlock(volume, NULL, (uint64_t)block_index);
            struct k_fs_pup_node_t *nodes = (struct k_fs_pup_node_t *)entry->buffer;
            uint32_t node_count = (1 << pup_volume->root.block_size_shift) / sizeof(struct k_fs_pup_node_t);

            for(uint32_t node_index = 0; node_index < node_count; node_index++)
            {
                if(nodes[node_index].type == K_FS_PUP_NODE_TYPE_NONE)
                {
                    nodes[node_index].type = type;
                    link = K_FS_PUP_MAKE_LINK(block_index, node_index, pup_volume->root.node_index_bits);

                    if(node_index == node_count - 1)
                    {
                        /* last node in this block, so mark it as full */
                        k_fs_PupSetRangeStatus(volume, &node_block, K_FS_PUP_BLOCK_STATUS_FULL);
                    }

                    break;
                }
            }
        }
    }

    return link;
}

void k_fs_PupFreeNode(struct k_fs_pup_link_t link)
{

}

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node)
{
    struct k_fs_pup_cset_t cache = {};
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_link_t node_link = k_fs_PupFindNode(volume, path, start_node, &cache);
    struct k_fs_pup_dirlist_t *dir_list = NULL;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    
    if(node_link.link)
    {
        struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, node_link, &cache);
        /* "."" and ".."" */
        uint32_t entry_count = 2 + node->child_count;
        dir_list = k_rt_Malloc(sizeof(struct k_fs_pup_dirlist_t) + sizeof(struct k_fs_pup_dirent_t) * entry_count, 4);
        
        k_rt_StrCpy(dir_list->entries[0].name, sizeof(dir_list->entries[0].name), ".");
        dir_list->entries[0].node = node_link;
        k_rt_StrCpy(dir_list->entries[1].name, sizeof(dir_list->entries[1].name), "..");
        dir_list->entries[1].node = node->parent;
        
        dir_list->used_count = 2;
        
        for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
        {
            struct k_fs_pup_range_t *range = node->ranges + range_index;
            // uint32_t first_block = K_FS_PUP_RANGE_START(range->contents);
            uint32_t first_block = (uint32_t)range->start;
            
            if(first_block)
            {
                struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, &cache, first_block);
                uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * block_size);
                
                if(entry_list->used_count)
                {
                    // dir_list = k_rt_Malloc(sizeof(struct k_fs_pup_dirlist_t) + sizeof(struct k_fs_pup_dirent_t) * entry_list->used_count, 4);
                    // dir_list->used_count = entry_list->used_count;
                    k_rt_CopyBytes(dir_list->entries + dir_list->used_count, entry_list->entries, sizeof(struct k_fs_pup_dirent_t ) * entry_list->used_count);
                    dir_list->used_count += entry_list->used_count;
                    break;
                }
            }
        }

        struct k_fs_pup_centry_t *entry = cache.first_entry;

        while(entry)
        {
            struct k_fs_pup_centry_t *next_entry = entry->next;
            k_fs_PupFreeCacheEntry(volume, entry);
            entry = next_entry;
        }
    }
    
    return dir_list;
}

uint32_t k_fs_PupGetPathToNode_r(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size, struct k_fs_pup_cset_t *cache)
{    
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_size = 1 << pup_volume->root.block_size_shift;

    if(start_node.link == pup_volume->root.root_node.link)
    {
        path_buffer[0] = '\0';
        k_rt_StrCpy(path_buffer, buffer_size, "/");
        return buffer_size - 1;
    }
    else
    {
        struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, start_node, cache);
        uint32_t available_buffer_size = k_fs_PupGetPathToNode_r(volume, node->parent, path_buffer, buffer_size, cache);
        
        struct k_fs_pup_node_t *parent_node = k_fs_PupGetNode(volume, node->parent, cache);
        for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
        {
            struct k_fs_pup_range_t *range = parent_node->ranges + range_index;
            // uint32_t first_block = K_FS_PUP_RANGE_START(range->contents);
            uint32_t first_block = (uint32_t)range->start;
            
            if(first_block)
            {
                struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, cache, first_block);
                uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * block_size);
                
                for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
                {
                    struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index;
                    
                    if(entry->node.link == start_node.link)
                    {
                        k_rt_StrCat(path_buffer, available_buffer_size, entry->name);
                        available_buffer_size -= k_rt_StrLen(entry->name) + 1;
                        k_rt_StrCat(path_buffer, available_buffer_size, "/");
                        available_buffer_size--;
                        return available_buffer_size;
                    }
                }
            }
        }
        // k_rt_StrCat(path_buffer, available_buffer_size, "/");
        // k_rt_StrCat(path_buffer, no)
    }
}

void k_fs_PupGetPathToNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size)
{
    struct k_fs_pup_cset_t cache = {};
    struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, start_node, &cache);
    // k_sys_TerminalPrintf("k_fs_PupGetPathToNode: %x\n", node);
    if(node)
    {
        k_fs_PupGetPathToNode_r(volume, start_node, path_buffer, buffer_size, &cache);
        
        struct k_fs_pup_centry_t *entry = cache.first_entry;
        
        while(entry)
        {
            struct k_fs_pup_centry_t *next_entry = entry->next;
            k_fs_PupFreeCacheEntry(volume, entry);
            entry = next_entry;
        }
    }
    // k_sys_TerminalPrintf("k_fs_PupGetPathToNode: %s\n", path_buffer);
}

void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
{
    
}

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
    
}