#include "pup.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../rt/string.h"
#include "../dsk/dsk.h"
#include "../sys/term.h"
#include "../mem/pmap.h"
#include "../cpu/k_cpu.h"
#include "fs.h"

// uint32_t k_fs_pup_block_size;
// uint8_t *k_fs_pup_disk_buffer;
// struct k_fs_pup_root_t *k_fs_pup_root;

void k_fs_PupMountVolume(struct k_fs_vol_t *volume)
{
    volume->data = k_rt_Malloc(sizeof(struct k_fs_pup_volume_t), 4);
    // volume->data = k_rt_Malloc(1024, 4);
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    uint32_t root_address = volume->partition.first_block * volume->partition.disk->block_size;
    
    k_dsk_Read(volume->partition.disk, root_address, sizeof(struct k_fs_pup_root_t), &pup_volume->root);
    // k_dsk_Read(volume->partition.disk, root_address, 512, &pup_volume->root);
    
    pup_volume->lru_bitmask = 0;
    pup_volume->entry_pool = NULL;
    
    for(uint32_t set_index = 0; set_index < K_FS_PUP_CACHE_SET_COUNT; set_index++)
    {
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        set->first_entry = NULL;
        set->last_entry = NULL;
        set->read_count = 0;
        set->write_lock = 0;
    }
    
    // k_sys_TerminalPrintf("pup volume mounted, block size: %d bytes\n", pup_volume->root.block_size);
}

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume)
{
    (void)volume;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupTouchEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry)
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

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_vol_t *volume)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_centry_t *entry = NULL;
    
    while(!entry)
    {
        // struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
        // entry = pup_volume->entry_pool;
        // 
        // while(entry && !k_rt_CmpXchg((uintptr_t *)pool_head, (uintptr_t)entry, (uintptr_t)entry->next, (uintptr_t *)&entry));
        
        if(!entry)
        {
            /* no entry in the pool */        
            if(pup_volume->allocated_memory < K_FS_PUP_MAX_CACHE_MEM)
            {
                /* we're still under the memory budget for this disk, so malloc a new cache entry */
                uint32_t alloc_size = sizeof(struct k_fs_pup_centry_t) + pup_volume->root.block_size * (K_FS_PUP_BLOCKS_PER_ENTRY);
                // k_sys_TerminalPrintf("entry size: %d\n", alloc_size);
                /* FIXME: race condition when modifying allocated_memory */
                pup_volume->allocated_memory += alloc_size;
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
    
    return entry;
}

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_centry_t **pool_head = &pup_volume->entry_pool;
    
    entry->next = pup_volume->entry_pool;
    
    while(!k_rt_CmpXchg((uintptr_t *)pool_head, (uintptr_t)entry->next, (uintptr_t)entry, (uintptr_t *)&entry->next))
    {
        /* nothing to see here */
    }
}

void k_fs_PupCacheEntry(struct k_fs_pup_volume_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry)
{
    /* FIXME: there's a race condition between the time a thread requests a line from disk
    and when it get inserted in the cache. If another thread tries to read the same location
    it won't find the line in cache and will also request it. Insertion in the cache should
    check for duplicated entries */
    
    // if(!set)
    // {
    //     struct k_fs_pup_cset_t *set = volume->cache_sets;
    // }
    
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

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSetWithCopyFields(struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end, struct k_fs_pup_centry_copy_t *copy)
{
    struct k_fs_pup_centry_t *entry = set->first_entry;
    uint32_t block_buffer_offset = 0;
    uint32_t entry_buffer_offset = 0;
    uint32_t copy_size = 0;
    
    // k_sys_TerminalPrintf("search entry for %x - %x on set %x\n", block_start, block_end, set);
    
    while(entry)
    {
        uint32_t entry_end = entry->first_block + K_FS_PUP_BLOCKS_PER_ENTRY;
        // k_sys_TerminalPrintf("test entry %x (%x - %x)\n", entry, entry->first_block, entry_end);
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
            entry = entry->next;
            continue;
        }
        
        if(copy)
        {
            copy->entry_buffer_offset = entry_buffer_offset;
            copy->data_buffer_offset = block_buffer_offset;
            copy->copy_size = copy_size;
        }
        
        break;
    }
    
    // k_sys_TerminalPrintf("entry %x\n", entry);
    
    return entry;
}

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSet(struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end)
{
    return k_fs_PupFindEntryInSetWithCopyFields(set, block_start, block_end, NULL);
}

struct k_fs_pup_centry_t *k_fs_PupDropOldestEntry(struct k_fs_vol_t *volume)
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
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    uint32_t block_end = block_start + block_count;
    uint8_t *block_buffer = (uint8_t *)buffer;
    uint32_t buffer_cursor = 0;
    // uint32_t request_ranges_count = 0;
    // struct k_fs_pup_range_t request_ranges[4];
    
    while(block_start < block_end)
    {
        // uint32_t set_index = K_FS_PUP_CACHE_SET_INDEX(block_start);
        uint32_t set_index = 0;
        struct k_fs_pup_cset_t *set = pup_volume->cache_sets + set_index;
        
        k_rt_Inc32Wrap(&set->read_count);
        k_rt_SpinWait(&set->write_lock);
        
        struct k_fs_pup_centry_copy_t copy = {};
        struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSetWithCopyFields(set, block_start, block_end, &copy);
        
        k_fs_PupTouchEntry(pup_volume, set_index, entry);
        k_rt_Dec32Wrap(&set->read_count);
        
        if(!entry)
        {
            /* entry not in cache, so load it from disk */
            uint32_t first_block = block_start & (~(K_FS_PUP_BLOCKS_PER_ENTRY - 1));
            uint32_t block_count = K_FS_PUP_BLOCKS_PER_ENTRY;
            entry = k_fs_PupAllocCacheEntry(volume);
            entry->first_block = first_block;
            
            copy.entry_buffer_offset = block_start % K_FS_PUP_BLOCKS_PER_ENTRY;
            copy.data_buffer_offset = buffer_cursor;
            copy.copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - copy.entry_buffer_offset;
            
            k_fs_ReadVolume(volume, pup_volume->root.block_size, first_block, block_count, entry->buffer);
            k_fs_PupCacheEntry(pup_volume, set, entry);
        }
        
        if(copy.copy_size > block_count)
        {
            copy.copy_size = block_count;
        }
        
        copy.data_buffer_offset *= pup_volume->root.block_size;
        copy.entry_buffer_offset *= pup_volume->root.block_size;

        k_rt_CopyBytes(block_buffer + copy.data_buffer_offset, entry->buffer + copy.entry_buffer_offset, copy.copy_size * pup_volume->root.block_size);
        
        block_start += copy.copy_size;
        buffer_cursor += copy.copy_size;
        block_count -= copy.copy_size;
    }
    
}

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer)
{
    
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_pup_centry_t *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint64_t block_address)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_centry_t *entry = k_fs_PupFindEntryInSet(set, (uint32_t)block_address, (uint32_t)block_address + 1);
    
    if(!entry)
    {
        // k_sys_TerminalPrintf("no entry for block %x\n", (uint32_t)block_address);
        // block_address &= ~(K_FS_PUP_BLOCKS_PER_ENTRY - 1);
        entry = k_fs_PupAllocCacheEntry(volume);
        entry->first_block = (uint32_t)block_address;    
        entry->first_block &= ~(K_FS_PUP_BLOCKS_PER_ENTRY - 1);
        // k_sys_TerminalPrintf("balls\n");
        k_fs_PupRead(volume, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
        
        
        k_fs_PupCacheEntry(pup_volume, set, entry);
        // k_sys_TerminalPrintf("cache entry %x\n", entry);
    }
    
    return entry;
}

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node, struct k_fs_pup_cset_t *cache)
{
    // struct k_fs_pup_node_t *cur_node;
    struct k_fs_pup_node_t *node = NULL;
    struct k_fs_pup_link_t node_link = {};
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_cset_t local_cache = {};
    uint32_t path_fragment_cursor = 0;
    uint32_t path_cursor = 0;
    char path_fragment[256];
    
    if(!cache)
    {
        cache = &local_cache;
    }
    
    if(!start_node.link && path[path_cursor] == '/')
    {
        path_cursor++;
        node = k_fs_PupGetNode(volume, pup_volume->root.root_node, cache);
        node_link = pup_volume->root.root_node;
    }
    else
    {
        node = k_fs_PupGetNode(volume, start_node, cache);
        node_link = start_node;
    }
        
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
        
        // k_sys_TerminalPrintf("%s\n", path_fragment);
        struct k_fs_pup_node_t *next_node = NULL;
        struct k_fs_pup_link_t next_node_link = {};
        
        // k_sys_TerminalPrintf("node type: %d\n", node->type);
        
        if(node->type == K_FS_PUP_NODE_TYPE_DIR)
        {                
            for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES && !next_node; range_index++)
            {
                struct k_fs_pup_range_t *range = node->ranges + range_index;
                uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
                
                // k_sys_TerminalPrintf("%s\n", path_fragment);
                
                if(!first_block)
                {
                    continue;
                }
                
                struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, cache, first_block);
                uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * pup_volume->root.block_size);
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
    
    return node_link;
}

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node)
{
    struct k_fs_pup_cset_t cache = {};
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_link_t node_link = k_fs_PupFindNode(volume, path, start_node, &cache);
    struct k_fs_pup_dirlist_t *dir_list = NULL;
    
    // k_sys_TerminalPrintf("blah\n");
    if(node_link.link)
    {
        struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, node_link, &cache);
        for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
        {
            struct k_fs_pup_range_t *range = node->ranges + range_index;
            uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
            
            if(first_block)
            {
                struct k_fs_pup_centry_t *block = k_fs_PupGetBlock(volume, &cache, first_block);
                uint32_t dir_entry_list_block = first_block % K_FS_PUP_BLOCKS_PER_ENTRY;
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(block->buffer + dir_entry_list_block * pup_volume->root.block_size);
                
                if(entry_list->used_count)
                {
                    dir_list = k_rt_Malloc(sizeof(struct k_fs_pup_dirlist_t) + sizeof(struct k_fs_pup_dirent_t) * entry_list->used_count, 4);
                    dir_list->used_count = entry_list->used_count;
                    k_rt_CopyBytes(dir_list->entries, entry_list->entries, sizeof(struct k_fs_pup_dirent_t ) * dir_list->used_count);
                    break;
                }
            }
        }
    }
    
    return dir_list;
}

struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_cset_t *cache)
{
    struct k_fs_pup_volume_t *pup_volume = (struct k_fs_pup_volume_t *)volume->data;
    struct k_fs_pup_node_t *node = NULL;
    uint32_t node_block = ((uint32_t)node_address.link) >> pup_volume->root.node_index_shift;
    uint32_t node_index_mask = (1 << pup_volume->root.node_index_shift) - 1;
    uint32_t node_index = ((uint32_t)node_address.link) & node_index_mask;
    
    /* offset relative to the start of the cache entry */
    node_block %= K_FS_PUP_BLOCKS_PER_ENTRY;
    
    // k_sys_TerminalPrintf("%d %d\n", node_block, node_index);
    
    struct k_fs_pup_centry_t *entry = k_fs_PupGetBlock(volume, cache, node_block);
    struct k_fs_pup_node_t *node_list = (struct k_fs_pup_node_t *)(entry->buffer + node_block * pup_volume->root.block_size);
    node = node_list + node_index;
    
    return node;
}

void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
{
    
}

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
    
}