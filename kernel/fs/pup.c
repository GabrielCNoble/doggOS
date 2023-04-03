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
    // volume->data = k_rt_Malloc(sizeof(struct k_fs_pup_vol_t), 4);
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // k_fs_ReadVolumeBytes(volume, volume->partition.disk->block_size, 0, 0, sizeof(struct k_fs_pup_root_t), &pup_volume->root); 

    // // for(uint32_t index = 0; index < K_FS_PUP_IDENT; index++)
    // // {
    // //     k_sys_TerminalPrintf("%c", pup_volume->root.ident[index]);
    // // }
    // // struct k_fs_pup_centry_t *entry = k_fs_PupAllocCacheEntry(pup_volume);
    // k_sys_TerminalPrintf("k_fs_PupMountVolume: root node: %x\n", (uint32_t)pup_volume->root.root_node.link);
    // // struct k_fs_pup_node_t root_node;
    // // k_fs_PupGetNode(volume, pup_volume->root.root_node, NULL, &root_node);
    // // k_sys_TerminalPrintf("%d\n", offsetof(struct k_fs_pup_node_t, parent)); 

    // uint32_t bitmask_block_count = (uint32_t)pup_volume->root.alloc_start.link - (uint32_t)pup_volume->root.bitmask_start.link;
    // // uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    // pup_volume->lru_bitmask = 0;
    // pup_volume->entry_pool = NULL;
    // pup_volume->block_bitmask = k_rt_Malloc(bitmask_block_count * K_FS_PUP_LOGICAL_BLOCK_SIZE, 4);

    // k_fs_ReadVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, (uint32_t)pup_volume->root.bitmask_start.link, bitmask_block_count, pup_volume->block_bitmask);

    // for(uint32_t set_index = 0; set_index < K_FS_PUP_CACHE_SET_COUNT; set_index++)
    // {
    //     struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
    //     set->first_entry = NULL;
    //     set->last_entry = NULL;
    //     set->read_count = 0;
    //     set->lock = 0;
    // }
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

    // uint32_t nodes_per_block = format_args->block_size / sizeof(struct k_fs_pup_node_t);
    // uint32_t mask = 0x80000000;
    // root.node_index_bits = 32;

    /* compute how many bits we need to store a node index */
    // while(mask)
    // {
    //     if(nodes_per_block & mask)
    //     {
    //         break;
    //     }

    //     mask >>= 1;
    //     root.node_index_bits--;
    // }

    // root.block_size_shift = 31;
    // mask = 0x80000000;
    // /* compute block size shift */
    // while(mask)
    // {
    //     if(format_args->block_size & mask)
    //     {
    //         break;
    //     }

    //     mask >>= 1;
    //     root.block_size_shift--;
    // }

    k_rt_StrCpy(root.ident, sizeof(root.ident), K_FS_PUP_MAGIC);
    root.bitmask_start.link = 1;
    /* each byte in a bitmask block contains state for 4 blocks, and we need
    1 block to contain the bitmask block. */
    uint32_t bitmask_block_count = format_args->block_count / (K_FS_PUP_LOGICAL_BLOCK_SIZE * 4 + 1);
    if(!bitmask_block_count)
    {
        bitmask_block_count++;
    }

    root.alloc_start.link = root.bitmask_start.link + bitmask_block_count;
    root.block_count = format_args->block_count;

    /* clear root block */
    k_fs_ClearVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, 0, 1);
    /* clear bitmask blocks */
    k_fs_ClearVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.bitmask_start.link, bitmask_block_count);

    /* allocate the first available block for nodes, and put the root node at the very beginning */
    uint8_t first_node_block = K_FS_PUP_BLOCK_TYPE_NODE;
    k_fs_WriteVolumeBytes(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.bitmask_start.link, 0, sizeof(first_node_block), &first_node_block);
    root.root_node = K_FS_PUP_NODE_LINK(root.alloc_start.link, 0);
    // k_sys_TerminalPrintf("node index bits: %d\n", (uint32_t)root.node_index_bits);
    /* write file system root */
    k_fs_WriteVolumeBytes(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, 0, 0, sizeof(struct k_fs_pup_root_t), &root);

    struct k_fs_pup_node_t root_node = {
        .type = K_FS_PUP_NODE_TYPE_DIR,
        .parent = root.root_node,   
    };
    /* initialize root node */
    k_fs_WriteVolumeBytes(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.alloc_start.link, 0, sizeof(struct k_fs_pup_node_t), &root_node);

    return 0;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupFlushCache(struct k_fs_pup_vol_t *volume)
{

}
 
void k_fs_PupTouchEntry(struct k_fs_pup_vol_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry)
{
    // if(entry)
    // {
    //     struct k_fs_pup_cset_t *set = volume->cache_sets + set_index;
    //     return;
        
    //     k_rt_SpinLock(&set->lock);
        
    //     if(entry->prev)
    //     { 
    //         entry->prev->next = entry->next;
            
    //         if(entry->next)
    //         {
    //             entry->next->prev = entry->prev;
    //         }
    //         else
    //         {
    //             set->last_entry = set->last_entry->prev;
    //         }
            
    //         entry->next = set->first_entry;
    //         set->first_entry = entry;
    //     }
        
    //     k_rt_SpinUnlock(&set->lock);
    // }
}

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_pup_vol_t *volume)
{
    // struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    // struct k_fs_pup_centry_t *entry = NULL;
    // // uint32_t block_size = 1 << volume->root.block_size_shift;
    
    // while(!entry)
    // {
    //     // struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
    //     // entry = pup_volume->entry_pool;
    //     // 
    //     // while(entry && !k_rt_CmpXchg((uintptr_t *)pool_head, (uintptr_t)entry, (uintptr_t)entry->next, (uintptr_t *)&entry));
        
    //     if(!entry)
    //     {
    //         // k_sys_TerminalPrintf("%x\n", volume);
    //         // k_cpu_DisableInterrupts();
    //         // k_cpu_Halt();
    //         /* no entry in the pool */        
    //         if(volume->allocated_memory < K_FS_PUP_MAX_CACHE_MEM)
    //         {
                
    //             /* we're still under the memory budget for this disk, so malloc a new cache entry */
    //             uint32_t alloc_size = sizeof(struct k_fs_pup_centry_t) + K_FS_PUP_LOGICAL_BLOCK_SIZE * (K_FS_PUP_BLOCKS_PER_ENTRY);
    //             // k_sys_TerminalPrintf("entry size: %d\n", alloc_size);
    //             /* FIXME: race condition when modifying allocated_memory */
    //             volume->allocated_memory += alloc_size;
    //             entry = k_rt_Malloc(alloc_size, 4);
    //         }
    //         else
    //         {
    //             /* well, crap. We're over budget here, so keep evicting the oldest entry 
    //             in the cache and tring until we get one */
    //             entry = k_fs_PupDropOldestEntry(volume);
    //         }
            
    //         // k_sys_TerminalPrintf("blah\n");
    //     }
    // }
    
    // entry->next = NULL;
    // entry->prev = NULL;
    // entry->ref_count = 0;
    // entry->lock = 0;
    // entry->init_lock = 0;
    
    // return entry;
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

// uint32_t k_fs_PupCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry)
// {    
//     uint32_t already_present = 0;

//     if(!set)
//     {
//         set = &volume->cache_sets[0];
//     }

//     /* lock the entry so any new threads trying to read it waits until this write is done */
//     k_rt_SpinLock(&set->lock);
//     /* wait for threads that are already reading from this entry before beginning a write */
//     while(!k_rt_CmpXchg32(&set->read_count, 0, 0, NULL));

//     set->cur_insert = entry;    

//     if(!set->first_entry)
//     {
//         set->first_entry = entry;
//         // set->last_entry = entry;
//     }
//     else
//     {
//         struct k_fs_pup_centry_t *duplicate = set->first_entry;

//         while(duplicate)
//         {
//             if(duplicate->first_block == entry->first_block)
//             {
//                 /* entry already in cache */
//                 already_present = 1;
//                 break;
//             }

//             duplicate = duplicate->next;
//         }

//         if(!duplicate)
//         {
//             set->last_entry->next = entry;
//             entry->prev = set->last_entry;   
//         }
//     }
//     set->last_entry = entry;
//     set->cur_insert = NULL;
//     k_rt_SpinUnlock(&set->lock);

//     return already_present;
// }

struct k_fs_pup_centry_t *k_fs_PupCacheEntryAddress(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t address)
{
    // uint32_t already_present = 0;
    // struct k_fs_pup_centry_t *entry = NULL;
    // address = address & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));

    // if(!set)
    // {
    //     set = &volume->cache_sets[0];
    // }

    // /* lock the entry so any new threads trying to read it waits until this write is done */
    // k_rt_SpinLock(&set->lock);
    // /* wait for threads that are already reading from this entry before beginning a write */
    // while(!k_rt_CmpXchg32(&set->read_count, 0, 0, NULL));

    // // set->cur_insert = entry;    

    // // if(!set->first_entry)
    // // {
    // //     set->first_entry = entry;
    // //     // set->last_entry = entry;
    // // }
    // // else
    // // {
    // struct k_fs_pup_centry_t *duplicate = set->first_entry;

    // while(duplicate)
    // {
    //     if(duplicate->first_block == entry->first_block)
    //     {
    //         /* entry already in cache */
    //         entry = duplicate;
    //         break;
    //     }

    //     duplicate = duplicate->next;
    // }

    // if(!duplicate)
    // {
    //     entry = k_fs_PupAllocCacheEntry(volume);
    //     entry->first_block = address;
    //     entry->ref_count = 1;
    //     entry->flags = K_FS_PUP_CENTRY_FLAG_PENDING;
        
    //     if(set->first_entry == NULL)
    //     {
    //         set->first_entry = NULL;
    //     }
    //     else
    //     {
    //         set->last_entry->next = entry;
    //         entry->prev = set->last_entry;   
    //     }            
    // }
    // // }
    // set->last_entry = entry;
    // // set->cur_insert = NULL;
    // k_rt_SpinUnlock(&set->lock);

    // return entry;
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
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

    // if(!set)
    // {
    //     /* FIXME: compute the proper cache set for the requested block address */
    //     set = &pup_volume->cache_sets[0];
    // }
    // // k_sys_TerminalPrintf("k_fs_PupFindEntryInSetWithCopyFields: %x, %x\n", block_start, block_end);
    // // uint32_t block_buffer_offset = 0;
    // uint32_t entry_buffer_offset = 0;
    // uint32_t copy_size = 0;
    // // uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    
    // /* very briefly lock the entry, to make sure a possible write to this set only 
    // begins after the read count has been incremented. This lock is not held all the
    // way through the read to guarantee many threads can read it at the same time */
    // k_rt_SpinLockCritical(&set->lock);
    // k_rt_Inc32Wrap(&set->read_count);
    // k_rt_SpinUnlockCritical(&set->lock);

    // struct k_fs_pup_centry_t *entry = set->first_entry;

    // while(entry)
    // {
    //     uint32_t entry_end = entry->first_block + K_FS_PUP_BLOCKS_PER_ENTRY;
    //     // k_sys_TerminalPrintf("test entry %x (%x - %x)\n", entry, entry->first_block, entry_end);
    //     if(entry->first_block >= block_start && entry->first_block < block_end)
    //     {
    //         // block_buffer_offset = entry->first_block - block_start;
    
    //         if(entry_end >= block_end)
    //         {
    //             copy_size = block_end - entry->first_block;
    //         }
    //         else
    //         {
    //             copy_size = entry_end - entry->first_block;
    //         }


    //     }
    //     else if(block_start >= entry->first_block && block_start < entry_end)
    //     {
    //         entry_buffer_offset = block_start - entry->first_block;
    
    //         if(block_end >= entry_end)
    //         {
    //             copy_size = entry_end - block_start;
    //         }
    //         else
    //         {
    //             copy_size = block_end - block_start;
    //         }
    //     }
    //     else
    //     {
    //         entry = entry->next;
    //         continue;
    //     }
        
    //     // k_sys_TerminalPrintf("entry %x is usable\n", entry);
    //     if(copy)
    //     {
    //         copy->entry_buffer_offset = entry_buffer_offset;
    //         // copy->data_buffer_offset = block_buffer_offset;
    //         copy->copy_size = copy_size;

    //         // k_sys_TerminalPrintf("copy fields for entry %x\n", entry);
    //     }

    //     /* to make sure this entry doesn't get evicted until everyone is done with it */
    //     // k_rt_Inc32Wrap(&entry->ref_count);
    //     // k_fs_PupAcquireEntry(&entry);
        
    //     break;
    // }

    // // k_sys_TerminalPrintf("k_fs_PupFindEntryInSetWithCopyFields: entry %x\n", entry);
    // // k_fs_PupReleaseEntry(&entry);
    // /* give threads wanting to write to the cache a go ahead */ 
    // k_rt_Dec32Wrap(&set->read_count);
    
    // return entry;
}

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSet(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end)
{
    return k_fs_PupFindEntryInSetWithCopyFields(volume, set, block_start, block_end, NULL);
}

void k_fs_PupAcquireEntry(struct k_fs_pup_centry_t *entry)
{
    if(entry != NULL)
    {
        k_rt_Inc32Wrap(&entry->ref_count);
    }
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

struct k_fs_pup_centry_t *k_fs_PupLoadEntry(struct k_fs_vol_t *volume, uint64_t block_address, uint64_t block_count, struct k_fs_pup_centry_copy_t *copy)
{
    // struct k_fs_pup_centry_t *entry = NULL;
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // uint32_t set_index = 0;
    // struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;

    // entry = k_fs_PupFindEntryInSetWithCopyFields(volume, set, block_address, block_address + block_count, copy);

    // if(entry == NULL)
    // {
    //     /* entry not in cache, so load it from disk */

    //     /* alloc a new entry or return an already existing one */
    //     entry = k_fs_PupCacheEntryAddress(pup_volume, set, block_address);

    //     if(k_rt_TrySpinLock(&entry->init_lock))
    //     {
    //         /* entry exists, but no data has been loaded for it, and we're the lucky thread to actually
    //         issue the read */
    //         if(copy != NULL)
    //         {
    //             copy->entry_buffer_offset = block_address % K_FS_PUP_BLOCKS_PER_ENTRY;
    //             copy->copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - copy->entry_buffer_offset;
    //             if(copy->copy_size > block_count)
    //             {
    //                 copy->copy_size = block_count;
    //             }
    //         }
            
    //         // k_sys_TerminalPrintf("entry: %p\n", entry);
            
    //         k_fs_ReadVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
            
    //         k_rt_AtomicAnd32(&entry->flags, ~K_FS_PUP_CENTRY_FLAG_PENDING);
    //     }
    // }
    // k_rt_SpinLockCritical(&entry->lock);
    // k_fs_PupAcquireEntry(entry);
    // k_rt_SpinUnlockCritical(&entry->lock);

    // /* this entry is in cache but a read to it is pendding, so spin for a bit until the
    // pending flag is cleared */
    // while(entry->flags & K_FS_PUP_CENTRY_FLAG_PENDING);

    // return entry;
}

void k_fs_PupRead(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // uint32_t block_end = block_start + block_count;
    // uint8_t *block_buffer = (uint8_t *)buffer;
    // uint32_t buffer_cursor = 0;
    
    // while(block_start < block_end)
    // {
    //     struct k_fs_pup_centry_copy_t copy = {};
    //     struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, block_start, block_count, &copy);
        
    //     if(copy.copy_size > block_count)
    //     {
    //         copy.copy_size = block_count;
    //     }

    //     copy.entry_buffer_offset *= K_FS_PUP_LOGICAL_BLOCK_SIZE;
    //     k_rt_CopyBytes(block_buffer + buffer_cursor, entry->buffer + copy.entry_buffer_offset, copy.copy_size * K_FS_PUP_LOGICAL_BLOCK_SIZE);
    //     k_fs_PupReleaseEntry(entry);
        
    //     block_start += copy.copy_size;
    //     buffer_cursor += copy.copy_size * K_FS_PUP_LOGICAL_BLOCK_SIZE;
    //     block_count -= copy.copy_size;
    // }
    
}

void k_fs_PupStoreEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // k_rt_SpinLock(&entry->lock);
    // k_fs_WriteVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
    // k_rt_AtomicAnd32(&entry->flags, ~K_FS_PUP_CENTRY_FLAG_DIRTY);
    // k_rt_SpinUnlock(&entry->lock);
}

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // uint32_t block_end = block_start + block_count;
    // uint8_t *block_buffer = (uint8_t *)buffer;
    // uint32_t buffer_cursor = 0;
    
    // while(block_start < block_end)
    // {
    //     uint32_t set_index = 0;
    //     struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
    //     struct k_fs_pup_centry_copy_t copy = {};
    //     struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, block_start, block_count, &copy);

    //     copy.entry_buffer_offset *= K_FS_PUP_LOGICAL_BLOCK_SIZE;
        
    //     k_rt_SpinLock(&entry->lock);
    //     while(!k_rt_CmpXchg32(&entry->ref_count, 1, 1, NULL));
    //     k_rt_CopyBytes(entry->buffer + copy.entry_buffer_offset, block_buffer + buffer_cursor, copy.copy_size * K_FS_PUP_LOGICAL_BLOCK_SIZE);
    //     k_rt_AtomicOr32(&entry->flags, K_FS_PUP_CENTRY_FLAG_DIRTY);
    //     k_fs_PupReleaseEntry(entry);
    //     k_rt_SpinUnlock(&entry->lock);
        
    //     block_start += copy.copy_size;
    //     buffer_cursor += copy.copy_size * K_FS_PUP_LOGICAL_BLOCK_SIZE;
    //     block_count -= copy.copy_size;
    // }
}

void k_fs_PupFlushDirtyEntries(struct k_fs_vol_t *volume)
{
    
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
void *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry, struct k_fs_pup_link_t block)
{
    return entry->buffer + K_FS_PUP_LOGICAL_BLOCK_SIZE * (block.link - entry->first_block);
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // k_fs_PupRead(volume, block.link, 1, buffer);
    // void *block = NULL;

    // struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSet(volume, set, (uint32_t)block.link, (uint32_t)block.link + 1);

    // while(block == NULL)
    // {
    //     struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSet(volume, set, (uint32_t)block.link, (uint32_t)block.link + 1);
    //     if(entry == NULL)
    //     {
    //        k_fs_PupRead(volume, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, NULL); 
    //     }
    // }



    // if(!entry)
    // {
    //     // entry = k_fs_PupAllocCacheEntry(pup_volume);
    //     // entry->first_block = (uint32_t)block.link;    
    //     // entry->first_block &= ~(K_FS_PUP_BLOCKS_PER_ENTRY - 1);

    //     block = k_mem_
    //     k_fs_PupRead(volume, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
    //     // k_fs_PupCacheEntry(pup_volume, set, entry);
    //     // k_sys_TerminalPrintf("entry not cached...\n");
    // }

    // cached_block = entry->buffer + (block.link % K_FS_PUP_BLOCKS_PER_ENTRY) * K_FS_PUP_LOGICAL_BLOCK_SIZE;
    
    // return cached_block;
}

// struct k_fs_pup_range_t k_fs_PupAllocRange(struct k_fs_vol_t *volume, uint32_t count, uint32_t alloc_type)
// {
//     // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
//     // struct k_fs_pup_range_t range = {};
//     // uint32_t block_size = 1 << pup_volume->root.block_size_shift;
//     // uint32_t block_count = pup_volume->root.block_count - pup_volume->root.alloc_start;
//     // uint32_t test_type = K_FS_PUP_BLOCK_TYPE_FREE;
//     // uint32_t range_start = 0;
//     // uint32_t range_count = 0;

//     // /* TODO: refactor all of this to something less dumb. */
//     // switch(alloc_type)
//     // {
//     //     case K_FS_PUP_BLOCK_TYPE_DATA:
//     //         test_type = K_FS_PUP_BLOCK_TYPE_FREE;
//     //     break;

//     //     case K_FS_PUP_BLOCK_TYPE_NODE:
//     //         test_type = K_FS_PUP_BLOCK_TYPE_NODE;
//     //     break;
//     // }

//     // /* TODO: refactor this loop to something a bit more efficient. Adding a second
//     // loop inside, that iterates over a single byte will allow computing some stuff
//     // once for the iterations of this inner loop.
    
//     // It may also be possible to test a whole byte at once. This will be testing the
//     // status for four consecutive blocks at once. */
//     // for(uint32_t block_index = 0; block_index < block_count; block_index++)
//     // {   
//     //     /* 4 blocks per byte */
//     //     uint8_t bitmask = pup_volume->bitmask_blocks[block_index >> 2];
//     //     uint32_t bitmask_shift = (block_index % 4) << 1;
//     //     uint32_t status = bitmask_shift >> bitmask_shift;

//     //     if(status == test_type || status == K_FS_PUP_BLOCK_TYPE_FREE)
//     //     {
//     //         if(!range_start)
//     //         {
//     //             range_start = block_index;
//     //             range_count = 0;
//     //         }

//     //         range_count++;

//     //         if(range_count == count)
//     //         {
//     //             range = K_FS_PUP_RANGE_MAKE_RANGE(range_start + pup_volume->root.alloc_start, range_count);
//     //             break;
//     //         }
//     //     }
//     //     else
//     //     {
//     //         range_start = 0;
//     //     }
//     // }

//     // k_fs_PupSetRangeType(volume, &range, alloc_type);

//     // return range;
// }

// uint32_t k_fs_PupTryReallocRange(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t count, uint32_t alloc_type)
// {
//     // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
//     // uint32_t test_type;

//     // uint32_t range_count = K_FS_PUP_RANGE_COUNT(range->contents);
//     // uint32_t range_start = K_FS_PUP_RANGE_START(range->contents);

//     // switch(alloc_type)
//     // {
//     //     case K_FS_PUP_BLOCK_TYPE_DATA:
//     //         test_type = K_FS_PUP_BLOCK_TYPE_FREE;
//     //     break;

//     //     case K_FS_PUP_BLOCK_TYPE_NODE:
//     //         test_type = K_FS_PUP_BLOCK_TYPE_NODE;
//     //     break;
//     // }

//     // if(count > range_count)
//     // {
//     //     uint32_t realloc_count = count - range_count;
//     //     /* start searching at the end of the provided range */
//     //     uint32_t realloc_start = (range_start + range_count) - pup_volume->root.alloc_start;
//     //     uint32_t realloc_end = realloc_start + realloc_count;
//     //     uint32_t block_index = realloc_start;

//     //     for(; block_index < realloc_end; block_index++)
//     //     {
//     //         uint8_t bitmask = pup_volume->bitmask_blocks[block_index >> 2];
//     //         uint32_t bitmask_shift = (block_index % 4) << 1;
//     //         uint32_t status = bitmask >> bitmask_shift;

//     //         if(status != K_FS_PUP_BLOCK_TYPE_FREE && status != test_type)
//     //         {
//     //             return 0;
//     //         }
//     //     }

//     //     /* change the block status only for the new portion of the range */
//     //     struct k_fs_pup_range_t set_range = K_FS_PUP_RANGE_MAKE_RANGE(realloc_start + pup_volume->root.alloc_start, realloc_count);
//     //     k_fs_PupSetRangeType(volume, &set_range, alloc_type);
//     //     *range = K_FS_PUP_RANGE_MAKE_RANGE(range_start, count);
//     // }
//     // else if(count < range_count)
//     // {
//     //     /* TODO: free the extra blocks at the end of the range */
//     // }

//     // return 1;
// }

// void k_fs_PupSetRangeType(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t type)
// {
//     // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume;

//     // uint32_t first_block = (uint32_t)K_FS_PUP_RANGE_START(range->contents) - pup_volume->root.alloc_start;
//     // uint32_t block_count = (uint32_t)K_FS_PUP_RANGE_COUNT(range->contents);

//     // for(uint32_t block_index = first_block; block_index < block_count; block_index++)
//     // {
//     //     /* 4 blocks per byte */
//     //     uint8_t bitmask = pup_volume->bitmask_blocks[block_index >> 2];
//     //     uint32_t bitmask_shift = (block_index % 4) << 1;
//     //     pup_volume->bitmask_blocks[block_index >> 2] |= type << bitmask_shift;
//     // }
// }

// void k_fs_PupFreeRange(struct k_fs_pup_range_t *range)
// {

// }

struct k_fs_pup_link_t k_fs_PupAllocBlock(struct k_fs_vol_t *volume, uint32_t block_type)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_count = pup_volume->root.block_count - pup_volume->root.alloc_start.link;
    uint32_t test_type = K_FS_PUP_BLOCK_TYPE_FREE;
    struct k_fs_pup_link_t block = K_FS_PUP_NULL_LINK;

    /* TODO: refactor all of this to something less dumb. */
    switch(block_type)
    {
        case K_FS_PUP_BLOCK_TYPE_CONTENT:
            test_type = K_FS_PUP_BLOCK_TYPE_FREE;
        break;

        case K_FS_PUP_BLOCK_TYPE_NODE:
            test_type = K_FS_PUP_BLOCK_TYPE_NODE;
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
        uint8_t bitmask = pup_volume->block_bitmask[block_index >> 2];
        uint32_t bitmask_shift = (block_index & 0x3) << 1;
        uint32_t status = bitmask_shift >> bitmask_shift;

        if(status == test_type || status == K_FS_PUP_BLOCK_TYPE_FREE)
        {
            block.link = pup_volume->root.alloc_start.link + block_index;
            k_fs_PupSetBlockType(volume, block, block_type);
            break;
        }
    }

    return block;
}

struct k_fs_pup_link_t k_fs_PupFindBlockByType(struct k_fs_vol_t *volume, uint32_t block_type)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_count = pup_volume->root.block_count - pup_volume->root.alloc_start.link;
    struct k_fs_pup_link_t block = K_FS_PUP_NULL_LINK;

    for(uint32_t block_index = 0; block_index < block_count; block_index++)
    {   
        /* 4 blocks per byte */
        uint8_t bitmask = pup_volume->block_bitmask[block_index >> 2];
        uint32_t bitmask_shift = (block_index & 0x3) << 1;
        uint32_t type = bitmask_shift >> bitmask_shift;

        if(type == block_type)
        {
            block.link = pup_volume->root.alloc_start.link + block_index;
            break;
        }
    }

    return block;
}

uint32_t k_fs_PupSetBlockType(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block, uint32_t block_type)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

    if(block.link < pup_volume->root.block_count)
    {
        uint64_t bitmask_index = block.link - pup_volume->root.alloc_start.link;
        uint32_t bitmask_shift = (bitmask_index & 0x3) << 1;
        pup_volume->block_bitmask[bitmask_index >> 2] |= block_type << bitmask_shift;
        return 1;
    }

    return 0;
}

void k_fs_PupFreeBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block)
{
    k_fs_PupSetBlockType(volume, block, K_FS_PUP_BLOCK_TYPE_FREE);
}

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node, struct k_fs_pup_cset_t *cache)
{
    // struct k_fs_pup_node_t *cur_node;
    // struct k_fs_pup_node_t *node = NULL;
    // struct k_fs_pup_link_t node_link = K_FS_PUP_NULL_LINK;
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // /* local cache set, to keep entries alive while we're using them without needing
    // synchronization */
    // // struct k_fs_pup_cset_t local_cache = {};
    // uint32_t path_fragment_cursor = 0;
    // uint32_t path_cursor = 0;
    // // uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    // char path_fragment[256];

    // // struct k_fs_pup_centry_t *entry = k_fs_PupAllocCacheEntry(pup_volume);
    
    // // if(!cache)
    // // {
    // //     cache = &local_cache;
    // // }
    
    // while(path[path_cursor] == ' ' && path[path_cursor] != '\0')
    // {
    //     path_cursor++;
    // }

    // // k_sys_TerminalPrintf("%s\n", path);
    // if(path[path_cursor] == '/' && !start_node.link)
    // {
    //     path_cursor++;
    //     // node = k_fs_PupGetNode(volume, pup_volume->root.root_node,);
    //     start_node = pup_volume->root.root_node;
    // }
    // // k_sys_TerminalPrintf("k_fs_PupFindNode: start_node: %x\n", (uint32_t)start_node.link);
    // if(start_node.link)
    // {
    //     node_link = start_node;
    //     struct k_fs_pup_centry_t *node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(node_link), 1, NULL);
    //     node = k_fs_PupGetNode(volume, node_link, node_entry);
            
    //     while(path[path_cursor] && node)
    //     {    
    //         path_fragment_cursor = 0;
    //         while(path[path_cursor] != '/' && path[path_cursor])
    //         {
    //             path_fragment[path_fragment_cursor] = path[path_cursor];
    //             path_fragment_cursor++;
    //             path_cursor++;
    //         }
            
    //         if(path[path_cursor] == '/')
    //         {
    //             path_cursor++;
    //         }
            
    //         path_fragment[path_fragment_cursor] = '\0';
            
    //         struct k_fs_pup_node_t *next_node = NULL;
    //         struct k_fs_pup_link_t next_node_link = {};

    //         if(node->type == K_FS_PUP_NODE_TYPE_DIR)
    //         {
    //             if(!k_rt_StrCmp(path_fragment, "."))
    //             {
    //                 next_node = node;
    //                 next_node_link = node_link;
    //             }                
    //             else if(!k_rt_StrCmp(path_fragment, ".."))
    //             {
    //                 next_node_link = node->parent;
    //                 k_fs_PupReleaseEntry(node_entry);
    //                 node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(next_node_link), 1, NULL);
    //                 next_node = k_fs_PupGetNode(volume, node->parent, node_entry);
    //             }
    //             else
    //             {   
    //                 struct k_fs_pup_link_t node_contents = node->contents;

    //                 _get_node_contents:
                    
    //                 // k_fs_PupGetBlock(volume, cache, node_contents, entry->buffer + K_FS_PUP_LOGICAL_BLOCK_SIZE);
    //                 k_fs_PupReleaseEntry(node_entry);
    //                 node_entry = k_fs_PupLoadEntry(volume, node_contents.link, 1, NULL);
    //                 // struct k_fs_pup_content_t *content = (struct k_fs_pup_content_t *)entry->buffer;
    //                 struct k_fs_pup_content_t *content = k_fs_PupGetBlock(volume, node_entry, node_contents);
    //                 struct k_fs_pup_dirent_t *dir_entries = &content->dir_entries[0];
    //                 struct k_fs_pup_dirent_t *dir_entry = dir_entries;
                    
    //                 while(dir_entry != NULL)
    //                 {
    //                     if(dir_entry->name[0] != '\0')
    //                     {
    //                         int32_t result = k_rt_StrCmp(path_fragment, dir_entry->name);

    //                         if(result < 0)
    //                         {
    //                             dir_entry = dir_entries + dir_entry->left;
    //                         }
    //                         else if(result > 0)
    //                         {
    //                             dir_entry = dir_entries + dir_entry->right;
    //                         }
    //                         else
    //                         {
    //                             next_node_link = dir_entry->link;
    //                             k_fs_PupReleaseEntry(node_entry);
    //                             node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(next_node_link), 1, NULL);
    //                             next_node = k_fs_PupGetNode(volume, dir_entry->link, node_entry);
    //                             break;
    //                         }
    //                     }
    //                     else
    //                     {
    //                         node_contents = dir_entry->link;
    //                         goto _get_node_contents;
    //                     }
    //                 }
    //             }
    //         }
            
    //         node = next_node;
    //         node_link = next_node_link;
    //     }

    //     k_fs_PupReleaseEntry(node_entry);
    // }

    // // k_fs_PupFreeCacheEntry(volume, entry);
    
    // return node_link;
}

/*
================
k_fs_PupGetNode

gets a node based on its address from a cache entry. The entry must not be released while the node is accessed.
================
*/
struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_centry_t *entry)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // struct k_fs_pup_node_t *node = NULL;
    // struct k_fs_pup_link_t node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(node_address)};
    // uint32_t node_index = K_FS_PUP_NODE_INDEX(node_address);

    // struct k_fs_pup_node_t *node_list = k_fs_PupGetBlock(volume, entry, node_block);
    // node = node_list + node_index;
    
    // return node;
}

struct k_fs_pup_link_t k_fs_PupAddNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path, const char *name, uint32_t type)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // struct k_fs_pup_link_t parent = k_fs_PupFindNode(volume, path, start_node, NULL);

    // if(parent.link != 0)
    // {
    //     struct k_fs_pup_centry_t *parent_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(parent), 1, NULL);
    //     struct k_fs_pup_node_t *parent_node = k_fs_PupGetNode(volume, parent, parent_entry);
    //     uint32_t init_contents = 0;

    //     if(!parent_node->contents.link)
    //     {
    //         parent_node->contents = k_fs_PupAllocBlock(volume, K_FS_PUP_BLOCK_TYPE_CONTENT);
    //         init_contents = 1;
    //     }

    //     struct k_fs_pup_centry_t *contents_entry = k_fs_PupLoadEntry(volume, parent_node->contents, 1, NULL);
    //     struct k_fs_pup_content_t *contents = k_fs_PupGetBlock(volume, contents_entry, parent_node->contents);

    //     if(init_contents)
    //     {
    //         k_fs_PupInitContents(contents, K_FS_PUP_CONTENT_TYPE_DIR);
    //     }

    //     uint16_t next_entry_index = contents->header.first_used;
    //     struct k_fs_pup_dirent_t *prev_entry = NULL;
    //     int32_t compare_result = 0;
        
    //     while(next_entry_index != 0xffff)
    //     {
    //         prev_entry = contents->dir_entries + next_entry_index;

    //         compare_result = k_rt_StrCmp(prev_entry->name, name);

    //         if(compare_result < 0)
    //         {
    //             next_entry_index = (uint16_t)prev_entry->left;
    //         }
    //         else if(compare_result > 0)
    //         {
    //             next_entry_index = (uint16_t)prev_entry->right;
    //         }
    //         else if(prev_entry->name[0] == '\0')
    //         {
    //             /* this entry points at another content buffer, so test its left and
    //             right nodes before following the link to this other buffer */

    //             if(prev_entry->left != 0xff)
    //             {
    //                 struct k_fs_pup_dirent_t *entry = contents->dir_entries + prev_entry->left;
    //                 compare_result = k_rt_StrCmp(entry->name, name);

    //                 if(compare_result < 0)
    //                 {
    //                     next_entry_index = entry->left;
    //                     prev_entry = entry;
    //                     continue;
    //                 }
    //             }

    //             if(prev_entry->right != 0xff)
    //             {
    //                 struct k_fs_pup_dirent_t *entry = contents->dir_entries + prev_entry->right;
    //                 compare_result = k_rt_StrCmp(entry->name, name);

    //                 if(compare_result > 0)
    //                 {
    //                     next_entry_index = entry->right;
    //                     prev_entry = entry;
    //                     continue;
    //                 }
    //             }

    //             struct k_fs_pup_link_t contents_link = prev_entry->link;
    //             k_fs_PupReleaseEntry(contents_entry);
    //             contents_entry = k_fs_PupLoadEntry(volume, contents_link.link, 1, NULL);
    //             contents = k_fs_PupGetBlock(volume, contents_entry, contents_link);
    //             next_entry_index = contents->header.first_used;
    //         }
    //     }

    //     if(prev_entry == NULL)
    //     {
    //         /* no entries in this content buffer */

    //     }
    //     else
    //     {
    //         if(contents->header.next_free == 0xffff)
    //         {
    //             /* no free entries in this content buffer, so we'll allocate a new
    //             content buffer, move the prev entry stuff into it and then make the
    //             prev entry point to it */
    //             struct k_fs_pup_link_t new_content_buffer = k_fs_PupAllocBlock(volume, K_FS_PUP_BLOCK_TYPE_CONTENT);
    //             struct k_fs_pup_centry_t *new_content_entry = k_fs_PupLoadEntry(volume, new_content_buffer.link, 1, NULL);
    //             struct k_fs_pup_content_t *new_contents = k_fs_PupGetBlock(volume, new_content_entry, new_content_buffer);
    //             k_fs_PupInitContents(new_contents, K_FS_PUP_CONTENT_TYPE_DIR);

    //             struct k_fs_pup_dirent_t *first_entry = new_contents->dir_entries + new_contents->header.next_free;
    //             new_contents->header.first_used = new_contents->header.next_free;
    //             new_contents->header.next_free = first_entry->right;
    //             first_entry->left = 0xff;
    //             first_entry->right = 0xff;

    //             k_rt_StrCpy(first_entry->name, sizeof(first_entry->name), prev_entry->name);
    //             // first_entry->link = 
    //             // prev_entry->link
    //         }
            
    //         next_entry_index = contents->header.next_free; 
    //         struct k_fs_pup_dirent_t *new_entry = contents->dir_entries + next_entry_index;
    //         contents->header.next_free = new_entry->right;

    //         new_entry->left = 0xff;
    //         new_entry->right = 0xff;

    //         if(compare_result < 0)
    //         {
    //             prev_entry->left = next_entry_index;
    //         }
    //         else
    //         {
    //             prev_entry->right = next_entry_index;
    //         }

    //         contents_entry->flags |= K_FS_PUP_CENTRY_FLAG_DIRTY;
    //     }

    //     k_fs_PupReleaseEntry(contents_entry);
    //     k_fs_PupReleaseEntry(parent_entry);
    // }
}

void k_fs_PupRemoveNode(struct k_fs_vol_t *volume, const char *path)
{

}

struct k_fs_pup_link_t k_fs_PupAllocNode(struct k_fs_vol_t *volume, uint32_t type)
{
    // struct k_fs_pup_link_t link = K_FS_PUP_NULL_LINK;
    // uint32_t init_node_block = 0;

    // if(type != K_FS_PUP_NODE_TYPE_NONE)
    // {
    //     struct k_fs_pup_link_t node_block = k_fs_PupFindBlockByType(volume, K_FS_PUP_BLOCK_TYPE_NODE);
    //     struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

    //     if(node_block.link == 0)
    //     {
    //         init_node_block = 1;
    //         node_block = k_fs_PupAllocBlock(volume, K_FS_PUP_BLOCK_TYPE_NODE);

    //         if(node_block.link == 0)
    //         {
    //             /* well, that's a beeg problem... */
    //         }
    //     }
        
    //     struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, node_block.link, 1, NULL);
    //     k_rt_SpinLock(&entry->lock);

    //     // if(node_block.link != 0)
    //     // {
    //         // struct k_fs_pup_node_t *nodes = k_fs_PupGetBlock(volume, NULL, node_block);
    //         // struct k_fs_pup_centry_t *nodes_entry = k_fs_PupCacheEntryAddress(pup_volume, NULL, node_block.link);
    //     // struct k_fs_pup_node_t *nodes = entry->buffer + K_FS_PUP_LOGICAL_BLOCK_SIZE * (node_block.link - entry->first_block);
    //     struct k_fs_pup_node_t *node_list = k_fs_PupGetBlock(volume, entry, node_block);

    //     uint32_t node_count = K_FS_PUP_LOGICAL_BLOCK_SIZE / sizeof(struct k_fs_pup_node_t);

    //     if(init_node_block)
    //     {
    //         for(uint32_t node_index = 0; node_index < node_count; node_index++)
    //         {
    //             node_list[node_index].type = K_FS_PUP_NODE_TYPE_NONE;
    //         }
    //     }

    //     for(uint32_t node_index = 0; node_index < node_count; node_index++)
    //     {
    //         if(node_list[node_index].type == K_FS_PUP_NODE_TYPE_NONE)
    //         {
    //             node_list[node_index].type = type;
    //             link = K_FS_PUP_NODE_LINK(node_block.link, node_index);

    //             if(node_index == node_count - 1)
    //             {
    //                 /* last node in this block, so mark it as full */
    //                 k_fs_PupSetBlockType(volume, node_block, K_FS_PUP_BLOCK_TYPE_FULL);
    //             }

    //             break; 
    //         }
    //     }
    //     // }
        
    //     k_rt_AtomicOr32(&entry->flags, K_FS_PUP_CENTRY_FLAG_DIRTY);
    //     k_rt_SpinUnlock(&entry->lock);
    //     k_fs_PupReleaseEntry(entry);
    // }

    // return link;
}

void k_fs_PupFreeNode(struct k_fs_pup_link_t link)
{
    
}

void k_fs_PupInitContents(struct k_fs_pup_content_t *contents, uint32_t type)
{
    // contents->header.first_used = 0xffff;
    // contents->header.next_free = 0;

    // switch(type)
    // {
    //     case K_FS_PUP_CONTENT_TYPE_DATA:
    //         for(uint32_t index = 0; index < sizeof(contents->data_entries) / sizeof(contents->data_entries[0]); index++)
    //         {
    //             contents->data_entries[index].offset = 0;
    //             contents->data_entries[index].link.link = 0;
    //             contents->data_entries[index].left = 0xffff;
    //             contents->data_entries[index].right = index + 1;
    //         }
    //     break;

    //     case K_FS_PUP_CONTENT_TYPE_DIR:
    //         for(uint32_t index = 0; index < sizeof(contents->dir_entries) / sizeof(contents->dir_entries[0]); index++)
    //         {
    //             contents->dir_entries[index].right = index + 1;
    //             contents->dir_entries[index].name[0] = '\0';
    //             contents->dir_entries[index].left = 0xffff;
    //             contents->dir_entries[index].link.link = 0;
    //         }
    //     break;
    // }
}

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node)
{
    // struct k_fs_pup_cset_t cache = {};
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // struct k_fs_pup_link_t node_link = k_fs_PupFindNode(volume, path, start_node, &cache);
    // struct k_fs_pup_dirlist_t *dir_list = NULL;
    // uint32_t block_size = 1 << pup_volume->root.block_size_shift;
    
    // if(node_link.link)
    // {
    //     struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, node_link, &cache);
    //     /* "." and ".." */
    //     uint32_t entry_count = 2 + node->child_count;
    //     dir_list = k_rt_Malloc(sizeof(struct k_fs_pup_dirlist_t) + sizeof(struct k_fs_pup_dirent_t) * entry_count, 4);
        
    //     k_rt_StrCpy(dir_list->entries[0].name, sizeof(dir_list->entries[0].name), ".");
    //     dir_list->entries[0].node = node_link;
    //     k_rt_StrCpy(dir_list->entries[1].name, sizeof(dir_list->entries[1].name), "..");
    //     dir_list->entries[1].node = node->parent;
        
    //     dir_list->used_count = 2;
        
    //     for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
    //     {
    //         struct k_fs_pup_range_t *range = node->ranges + range_index;
    //         uint32_t first_block = K_FS_PUP_RANGE_START(range->contents);
    //         // uint32_t first_block = (uint32_t)range->start;
            
    //         if(first_block)
    //         {
    //             struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, &cache, first_block);
    //             uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
    //             struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * block_size);
                
    //             if(entry_list->used_count)
    //             {
    //                 // dir_list = k_rt_Malloc(sizeof(struct k_fs_pup_dirlist_t) + sizeof(struct k_fs_pup_dirent_t) * entry_list->used_count, 4);
    //                 // dir_list->used_count = entry_list->used_count;
    //                 k_rt_CopyBytes(dir_list->entries + dir_list->used_count, entry_list->entries, sizeof(struct k_fs_pup_dirent_t ) * entry_list->used_count);
    //                 dir_list->used_count += entry_list->used_count;
    //                 break;
    //             }
    //         }
    //     }

    //     struct k_fs_pup_centry_t *entry = cache.first_entry;

    //     while(entry)
    //     {
    //         struct k_fs_pup_centry_t *next_entry = entry->next;
    //         k_fs_PupFreeCacheEntry(volume, entry);
    //         entry = next_entry;
    //     }
    // }
    
    // return dir_list;
}

uint32_t k_fs_PupGetPathToNode_r(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size, struct k_fs_pup_cset_t *cache)
{    
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // // uint32_t block_size = 1 << pup_volume->root.block_size_shift;

    // if(start_node.link == pup_volume->root.root_node.link)
    // {
    //     path_buffer[0] = '\0';
    //     k_rt_StrCpy(path_buffer, buffer_size, "/");
    //     return buffer_size - 1;
    // }
    // else
    // {
    //     struct k_fs_pup_centry_t *node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(start_node), 1, NULL);
    //     struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, start_node, node_entry);
    //     uint32_t available_buffer_size = k_fs_PupGetPathToNode_r(volume, node->parent, path_buffer, buffer_size, cache);
    //     struct k_fs_pup_link_t parent_link = node->parent;

    //     k_fs_PupReleaseEntry(node_entry);
    //     entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(parent_link), 1, NULL);
    //     node = k_fs_PupGetNode(volume, parent_link, node_entry); 

        

    //     // start_node = node->parent;
    //     // k_fs_PupReleaseEntry(node_entry);
    //     // node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(start_node), 1, NULL);
    //     // struct k_fs_pup_node_t *parent_node = k_fs_PupGetNode(volume, start_node, node_entry);
    //     // for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
    //     // {
    //     //     struct k_fs_pup_range_t *range = parent_node->ranges + range_index;
    //     //     uint32_t first_block = K_FS_PUP_RANGE_START(range->contents);
    //     //     // uint32_t first_block = (uint32_t)range->start;
            
    //     //     if(first_block)
    //     //     {
    //     //         struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, cache, first_block);
    //     //         uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
    //     //         struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * block_size);
                
    //     //         for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
    //     //         {
    //     //             struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index;
                    
    //     //             if(entry->node.link == start_node.link)
    //     //             {
    //     //                 k_rt_StrCat(path_buffer, available_buffer_size, entry->name);
    //     //                 available_buffer_size -= k_rt_StrLen(entry->name) + 1;
    //     //                 k_rt_StrCat(path_buffer, available_buffer_size, "/");
    //     //                 available_buffer_size--;
    //     //                 return available_buffer_size;
    //     //             }
    //     //         }
    //     //     }
    //     // }
    //     // k_rt_StrCat(path_buffer, available_buffer_size, "/");
    //     // k_rt_StrCat(path_buffer, no)
    // }
}

void k_fs_PupGetPathToNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(start_node), 1, NULL);
    // struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, start_node, entry);
    
    // if(node != NULL)
    // {
    //     k_fs_PupGetPathToNode_r(volume, start_node, path_buffer, buffer_size, NULL);
    // }
    
    // k_fs_PupReleaseEntry(entry);
}

// void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
// {
    
// }

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
    
}