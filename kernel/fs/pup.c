#include "pup.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../rt/string.h"
#include "../dsk/dsk.h"
#include "../sys/term.h"
#include "../sys/defs.h"
#include "../sys/sys.h"
#include "../mem/pmap.h"
#include "../cpu/k_cpu.h"
#include "fs.h"

// uint32_t k_fs_pup_block_size;
// uint8_t *k_fs_pup_disk_buffer;
// struct k_fs_pup_root_t *k_fs_pup_root;

void k_fs_PupMountVolume(struct k_fs_vol_t *volume)
{
    volume->data = k_rt_Malloc(sizeof(struct k_fs_pup_vol_t), 4);
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    k_rt_SetBytes(volume->data, sizeof(struct k_fs_pup_vol_t), 0);
    // k_sys_TerminalPrintf("mounting volume...\n");
    pup_volume->cur_entry_page = k_rt_Malloc(sizeof(struct k_fs_pup_centry_page_t), 4);
    k_fs_PupInitCacheEntryPage(volume->data, pup_volume->cur_entry_page);
    // k_sys_TerminalPrintf("%x\n", volume->partition.disk);
    k_fs_ReadVolumeBytes(volume, volume->partition.disk->block_size, 0, 0, sizeof(struct k_fs_pup_root_t), &pup_volume->root);    
    
    

    // for(uint32_t index = 0; index < K_FS_PUP_IDENT; index++)
    // {
    //     k_sys_TerminalPrintf("%c", pup_volume->root.ident[index]);
    // }
    // k_sys_TerminalPrintf("\n");


    pup_volume->block_bitmask = k_rt_Malloc(pup_volume->root.bitmask_block_count * K_FS_PUP_LOGICAL_BLOCK_SIZE, 4);
    // k_sys_TerminalPrintf("%d bitmask blocks\n", (uint32_t)pup_volume->root.bitmask_block_count);
    k_fs_ReadVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, (uint32_t)pup_volume->root.bitmask_start.link, pup_volume->root.bitmask_block_count, pup_volume->block_bitmask);
    k_rt_SetBytes(pup_volume->cache_sets, sizeof(pup_volume->cache_sets), 0);

    struct k_fs_pup_link_t root_node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(pup_volume->root.root_node)};
    // k_sys_TerminalPrintf("root node block: %d\n", (uint32_t)root_node_block.link);
    struct k_fs_pup_centry_t *root_node_entry = k_fs_PupFindOrLoadCacheEntry(volume, root_node_block);
    // k_sys_TerminalPrintf("entry at: %x\n", &root_node_entry->buffer);
    union k_fs_pup_content_t *volume_contents = k_fs_PupGetBlock(volume, root_node_block, root_node_entry);
    // k_sys_TerminalPrintf("contents are at %x\n", volume_contents);
    struct k_fs_pup_node_t *root_node = volume_contents->dir.entries + K_FS_PUP_NODE_INDEX(pup_volume->root.root_node);

    // k_sys_TerminalPrintf("Root node name is: %s\n", root_node->name);
    // k_sys_TerminalPrintf("Root node contents are: %d\n", (uint32_t)root_node->contents.link);
}

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume)
{
    if (volume && volume->data)
    {
        struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
        // uint32_t bitmask_block_count = pup_volume->root.alloc_start.link - pup_volume->root.bitmask_start.link;
        k_fs_PupFlushCache(volume);

        k_fs_WriteVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, pup_volume->root.bitmask_start.link, pup_volume->root.bitmask_block_count, pup_volume->block_bitmask);
        k_rt_Free(volume->data);
        volume->data = NULL;
    }
}

uint32_t k_fs_PupFormatVolume(struct k_fs_vol_t *volume, void *args)
{
    struct k_fs_pup_format_args_t *format_args = (struct k_fs_pup_format_args_t *)args;
    struct k_fs_pup_root_t root = {};

    k_rt_StrCpy(root.ident, sizeof(root.ident), K_FS_PUP_MAGIC);
    root.root_node = K_FS_PUP_NODE_LINK(1, 0);
    root.bitmask_start.link = 2;
    /* each byte in a bitmask block contains state for 8 blocks, and we need
    1 block to contain the bitmask block. */
    root.bitmask_block_count = (format_args->block_count * K_FS_PUP_LOGICAL_BLOCK_SIZE) / (K_FS_PUP_LOGICAL_BLOCK_SIZE * 8 + 1);
    if(!root.bitmask_block_count)
    {
        root.bitmask_block_count++;
    }

    root.alloc_start.link = root.bitmask_start.link + root.bitmask_block_count;
    root.block_count = format_args->block_count;

    root.alloc_start.link = (root.alloc_start.link + 3) & (~3);

    /* clear root block */
    k_fs_ClearVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.root_node.link, 1);
    /* clear bitmask blocks */
    k_fs_ClearVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.bitmask_start.link, root.bitmask_block_count);

    // /* allocate the first available block for nodes, and put the root node at the very beginning */
    // uint8_t first_node_block = K_FS_PUP_BLOCK_TYPE_NODE;
    // k_fs_WriteVolumeBytes(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, root.bitmask_start.link, 0, sizeof(first_node_block), &first_node_block);
    // // root.root_node = K_FS_PUP_NODE_LINK(root.alloc_start.link, 0);
    // // k_sys_TerminalPrintf("node index bits: %d\n", (uint32_t)root.node_index_bits);


    /* write file system root */
    k_fs_WriteVolumeBytes(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, 0, 0, sizeof(struct k_fs_pup_root_t), &root);

    union k_fs_pup_content_t volume_contents;
    k_rt_SetBytes(&volume_contents, sizeof(union k_fs_pup_content_t), 0);

    /* root parent is itself */
    volume_contents.dir.parent = K_FS_PUP_NODE_LINK(1, 0);
    struct k_fs_pup_node_t *root_node = volume_contents.dir.entries;
    root_node->type = K_FS_PUP_NODE_TYPE_DIR;
    root_node->left = 0xffff;
    root_node->right = 0xffff;
    root_node->contents = K_FS_PUP_NULL_LINK;
    k_rt_StrCpy(root_node->name, sizeof(root_node->name), "I'm dat root, mofkr");
    /* write root node */
    k_fs_WriteVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, K_FS_PUP_NODE_BLOCK(root.root_node), 1, &volume_contents);

    return 0;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

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

void k_fs_PupInitCacheEntryPage(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_page_t *page)
{
    page->next = NULL;
    page->prev = NULL;

    struct k_fs_pup_centry_t *next_entry = NULL;

    for (uint32_t index = K_FS_PUP_CENTRY_PAGE_ENTRIES_COUNT; index > 0;)
    {
        index--;
        struct k_fs_pup_centry_t *entry = page->entries + index;
        entry->page = page;
        entry->next = next_entry;
        next_entry = entry;
    }

    page->free_entries = page->entries;
    page->index = volume->entry_page_count;
    page->used_entries = 0;
    volume->entry_page_count++;

    if (volume->entry_pages == NULL)
    {
        volume->entry_pages = page;
    }
    else
    {
        page->prev = volume->last_entry_page;
        volume->last_entry_page->next = page;
    }

    volume->last_entry_page = page;
}

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_pup_vol_t *volume)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_centry_t *entry = NULL;

    k_rt_SpinLock(&volume->entry_page_lock);

    struct k_fs_pup_centry_page_t *entry_page = volume->cur_entry_page;

    if (entry_page->free_entries == NULL)
    {
        while (entry_page && entry_page->free_entries == NULL)
        {
            entry_page = entry_page->next;
        }

        if (entry_page == NULL)
        {
            if (volume->freeable_entry_pages)
            {
                entry_page = volume->freeable_entry_pages;
                volume->freeable_entry_pages = volume->freeable_entry_pages->next;
            }
            else
            {
                entry_page = k_rt_Malloc(sizeof(struct k_fs_pup_centry_page_t), 4);

                if (entry_page == NULL)
                {
                    k_sys_RaiseException(K_EXCEPTION_FAILED_MEMORY_ALLOCATION);
                }
            }

            k_fs_PupInitCacheEntryPage(volume, entry_page);
        }

        volume->cur_entry_page = entry_page;
    }

    entry = entry_page->free_entries;
    entry_page->free_entries = entry_page->free_entries->next;
    entry_page->used_entries++;

    k_rt_SpinUnlock(&volume->entry_page_lock);

    entry->next = NULL;
    entry->prev = NULL;
    entry->ref_count = 0;
    entry->lock = 0;
    entry->init_lock = 0;

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

void k_fs_PupFreeCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    if (volume != NULL && entry != NULL)
    {
        k_rt_SpinLock(&volume->entry_page_lock);

        struct k_fs_pup_centry_page_t *page = entry->page;

        entry->next = page->free_entries;
        page->free_entries = entry;
        page->used_entries--;

        if (page->used_entries == 0 && volume->entry_page_count > 1)
        {
            struct k_fs_pup_centry_page_t *used_page = page->next;
            uint32_t page_index = page->index;

            while (used_page)
            {
                used_page->index = page_index;
                page_index++;
                used_page = used_page->next;
            }

            if (page == volume->entry_pages)
            {
                volume->entry_pages = page->next;
                volume->entry_pages->prev = NULL;
            }
            else
            {
                page->prev->next = page->next;
            }

            if (page == volume->last_entry_page)
            {
                volume->last_entry_page = page->prev;
                volume->last_entry_page->next = NULL;
            }
            else
            {
                page->next->prev = page->prev;
            }

            page->next = volume->freeable_entry_pages;
            volume->freeable_entry_pages = page;
            volume->entry_page_count--;

            if (page == volume->cur_entry_page)
            {
                volume->cur_entry_page = volume->entry_pages;
            }
        }
        else if (page->index < volume->cur_entry_page->index)
        {
            volume->cur_entry_page = page;
        }

        k_rt_SpinUnlock(&volume->entry_page_lock);
    }
}

void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_cset_t *set = &pup_volume->cache_sets[0];

    struct k_fs_pup_centry_t *entry = set->first_entry;
    while(entry)
    {
        k_fs_PupStoreCacheEntry(volume, entry);
        entry = entry->next;
    }
}

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

struct k_fs_pup_centry_t *k_fs_PupFindCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_cset_t *set = &pup_volume->cache_sets[0];

    /* 
        very briefly lock this set. This is necessary to make sure threads wanting to read will
        do so only when threads writing are done, and vice versa. It uses the critical variant
        to avoid having the thread getting preempted after acquiring the lock.
    */
    k_rt_SpinLockCritical(&set->lock);
    k_rt_Inc32Wrap(&set->read_count);
    k_rt_SpinUnlockCritical(&set->lock);

    struct k_fs_pup_centry_t *entry = set->first_entry;

    while (entry)
    {
        uint64_t entry_end = entry->first_block.link + (K_FS_PUP_BLOCKS_PER_ENTRY - 1);
        // k_sys_TerminalPrintf("block: %d, entry: %d - %d\n", (uint32_t)block.link, (uint32_t)entry->first_block.link, (uint32_t)entry_end);
        if (block.link < entry->first_block.link || block.link > entry_end)
        {
            entry = entry->next;
            continue;
        }

        break;
    }

    k_fs_PupAcquireEntry(entry);

    /* we're done with this set */
    k_rt_Dec32Wrap(&set->read_count);

    return entry;
}

struct k_fs_pup_centry_t *k_fs_PupFindOrLoadCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_centry_t *entry = k_fs_PupFindCacheEntry(volume, block);

    if (entry == NULL)
    {
        struct k_fs_pup_cset_t *set = &pup_volume->cache_sets[0];

        if (k_rt_TrySpinLock(&set->lock))
        {
            // k_sys_TerminalPrintf("no entry for block %d\n", (uint32_t)block.link);
            entry = k_fs_PupAllocCacheEntry(pup_volume);
            entry->flags |= K_FS_PUP_CENTRY_FLAG_PENDING;
            entry->first_block.link = block.link & 0xfffffffffffffffc;
            // k_sys_TerminalPrintf("created entry %d - %d\n", (uint32_t)entry->first_block.link, (uint32_t)entry->first_block.link + K_FS_PUP_BLOCKS_PER_ENTRY);
            k_fs_PupStashCacheEntry(pup_volume, entry);
            k_rt_SpinUnlock(&set->lock);
            k_fs_ReadVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, entry->first_block.link, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
            entry->flags &= ~K_FS_PUP_CENTRY_FLAG_PENDING;
            k_rt_SignalCondition(&entry->condition);

            // struct k_fs_pup_centry_t *entry = pup_volume->cache_sets[0].first_entry;
            // k_sys_TerminalPrintf("test entries\n");
            // while(entry)
            // {
            //     k_sys_TerminalPrintf("entry: %d - %d\n", (uint32_t)entry->first_block.link, (uint32_t)entry->first_block.link + K_FS_PUP_BLOCKS_PER_ENTRY);
            //     entry = entry->next;
            // }
        }
        else
        {
            k_rt_SpinWait(&set->lock);
            entry = k_fs_PupFindCacheEntry(volume, block);
        }
    }

    if (entry->flags & K_FS_PUP_CENTRY_FLAG_PENDING)
    {
        k_proc_WaitCondition(&entry->condition);
    }

    return entry;
}

void k_fs_PupStoreCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_cset_t *set = &pup_volume->cache_sets[0];

    // k_rt_SpinLockCritical(&set->lock);
    // k_rt_Inc32Wrap(&set->read_count);
    // k_rt_SpinUnlockCritical(&set->lock);

    if(entry != NULL /* && (entry->flags & K_FS_PUP_CENTRY_FLAG_DIRTY) */)
    {
        k_fs_WriteVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, entry->first_block.link, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
        entry->flags &= ~K_FS_PUP_CENTRY_FLAG_DIRTY;
    }
    // k_rt_SpinLock(&entry->lock);
    // k_fs_WriteVolumeBlocks(volume, K_FS_PUP_LOGICAL_BLOCK_SIZE, entry->first_block, K_FS_PUP_BLOCKS_PER_ENTRY, entry->buffer);
    // k_rt_AtomicAnd32(&entry->flags, ~K_FS_PUP_CENTRY_FLAG_DIRTY);
    // k_rt_SpinUnlock(&entry->lock);
}

void k_fs_PupStashCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_t *entry)
{
    // struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_cset_t *set = &volume->cache_sets[0];

    // k_rt_SpinLock(&set->lock);

    /* wait for anyone that might be reading from this set */
    while (set->read_count != 0);

    if(set->first_entry == NULL)
    {
        set->first_entry = entry;
    }
    else
    {
        set->last_entry->next = entry;
    }
    
    set->last_entry = entry;

    // k_rt_SpinUnlock(&set->lock);
}

void k_fs_PupAcquireEntry(struct k_fs_pup_centry_t *entry)
{
    if (entry != NULL)
    {
        k_rt_Inc32Wrap(&entry->ref_count);
    }
}

void k_fs_PupReleaseEntry(struct k_fs_pup_centry_t *entry)
{
    if (entry && entry->ref_count)
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

void k_fs_PupRead(struct k_fs_vol_t *volume, uint64_t first_block, uint32_t block_count, void *buffer)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;

    if(block_count > 0)
    {
        uint64_t last_block = first_block + block_count;
        uint8_t *block_buffer = (uint8_t *)buffer;
        uint32_t buffer_cursor = 0;

        while(first_block < last_block)
        {
            struct k_fs_pup_centry_t *entry = k_fs_PupFindOrLoadCacheEntry(volume, (struct k_fs_pup_link_t){first_block});

            uint32_t block_offset = first_block - entry->first_block.link;
            uint32_t copy_size = K_FS_PUP_BLOCKS_PER_ENTRY - block_offset;

            if(copy_size > block_count)
            {
                copy_size = block_count;
            }

            first_block += copy_size;
            block_count -= copy_size;

            block_offset *= K_FS_PUP_LOGICAL_BLOCK_SIZE;
            copy_size *= K_FS_PUP_LOGICAL_BLOCK_SIZE;
            k_rt_CopyBytes(block_buffer + buffer_cursor, entry->buffer + block_offset, copy_size);
            k_fs_PupReleaseEntry(entry);
            buffer_cursor += copy_size;
        }
    }
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

// void k_fs_PupFlushDirtyEntries(struct k_fs_vol_t *volume)
// {
// }

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block, struct k_fs_pup_centry_t *entry)
{
    uint32_t block_offset = (block.link - entry->first_block.link) * K_FS_PUP_LOGICAL_BLOCK_SIZE;
    // k_sys_TerminalPrintf("offset is: %d\n", block_offset);
    return entry->buffer + block_offset;
}

struct k_fs_pup_link_t k_fs_PupAllocBlock(struct k_fs_vol_t *volume)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t block_count = pup_volume->root.block_count - pup_volume->root.alloc_start.link;
    // uint32_t test_type = K_FS_PUP_BLOCK_TYPE_FREE;
    struct k_fs_pup_link_t block = K_FS_PUP_NULL_LINK;

    /* TODO: refactor this loop to something a bit more efficient. Adding a second
    loop inside, that iterates over a single byte will allow computing some stuff
    once for the iterations of this inner loop.

    It may also be possible to test a whole byte at once. This will be testing the
    status for four consecutive blocks at once. */

    struct k_fs_pup_centry_t *cur_entry;

    for (uint32_t block_index = 0; block_index < block_count; block_index++)
    {
        /* 8 blocks per byte */
        uint32_t bitmask_index = block_index >> 3;
        uint8_t bitmask = pup_volume->block_bitmask[bitmask_index];
        uint8_t bit = 1 << (block_index & 0x7);

        if (!(bitmask & bit))
        {
            if (k_rt_CmpXchg8(&pup_volume->block_bitmask[bitmask_index], bitmask, bitmask | bit, NULL))
            {
                block.link = pup_volume->root.alloc_start.link + block_index;
                break;
            }

            block_index = 0xffffffff;
        }
    }

    return block;
}

void k_fs_PupFreeBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint64_t bitmask_index = (block.link - pup_volume->root.alloc_start.link) >> 3;
    uint8_t bitmask = pup_volume->block_bitmask[bitmask_index];
    uint8_t bit = 1 << (bitmask_index & 0x7);

    while (!k_rt_CmpXchg8(&pup_volume->block_bitmask[bitmask_index], bitmask, bitmask & ~bit, NULL));
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node)
{
    struct k_fs_pup_node_t *node = NULL;
    struct k_fs_pup_link_t node_link = K_FS_PUP_NULL_LINK;
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    uint32_t path_fragment_cursor = 0;
    uint32_t path_cursor = 0;
    char path_fragment[256];


    while(path[path_cursor] == ' ' && path[path_cursor] != '\0')
    {
        path_cursor++;
    }

    if(!start_node.link)
    {
        start_node = pup_volume->root.root_node;
    }

    if(path[path_cursor] == '/')
    {
        path_cursor++;
    }

    // if(start_node.link)
    {
        node_link = start_node;
        struct k_fs_pup_link_t node_parent_contents_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(node_link)};
        struct k_fs_pup_centry_t *node_entry = k_fs_PupFindOrLoadCacheEntry(volume, node_parent_contents_block);
        union k_fs_pup_content_t *node_parent_contents = k_fs_PupGetBlock(volume, node_parent_contents_block, node_entry);
        node = k_fs_PupGetNode(volume, node_link, node_entry);
        // node = node_parent_contents->dir.entries + K_FS_PUP_NODE_INDEX(node_link);
        // node = k_fs_PupGetNode(volume, node_link, node_entry);

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
            struct k_fs_pup_link_t next_node_link = K_FS_PUP_NULL_LINK;
            struct k_fs_pup_link_t next_node_block = K_FS_PUP_NULL_LINK;

            if(node->type == K_FS_PUP_NODE_TYPE_DIR)
            {
                if(!k_rt_StrCmp(path_fragment, "."))
                {
                    next_node = node;
                    next_node_link = node_link;
                }
                else if(!k_rt_StrCmp(path_fragment, ".."))
                {
                    next_node_link = node_parent_contents->dir.parent;
                    node_parent_contents_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(next_node_link)};
                    k_fs_PupReleaseEntry(node_entry);
                    node_entry = k_fs_PupFindOrLoadCacheEntry(volume, node_parent_contents_block);
                    next_node = k_fs_PupGetNode(volume, next_node_link, node_entry);
                    node_parent_contents = k_fs_PupGetBlock(volume, node_parent_contents_block, node_entry);
                }
                else
                {

                    next_node = NULL;
                    next_node_link = K_FS_PUP_NULL_LINK;
                    struct k_fs_pup_link_t node_contents_block = node->contents;

                    if(node_contents_block.link != 0)
                    {
                        struct k_fs_pup_centry_t *node_contents_entry = k_fs_PupFindOrLoadCacheEntry(volume, node->contents);
                        // k_sys_TerminalPrintf("piss\n");
                        union k_fs_pup_content_t *node_contents = k_fs_PupGetBlock(volume, node->contents, node_contents_entry);
                        uint16_t child_node_index = node_contents->dir.first_used;


                        // k_sys_TerminalPrintf("contents: %d to %d\n", (uint32_t)node_contents_entry->first_block.link, (uint32_t)node_contents_entry->first_block.link + K_FS_PUP_BLOCKS_PER_ENTRY);
                        // k_sys_TerminalPrintf("first entry: %d, next free: %d\n", (uint32_t)node_contents->dir.first_used, (uint32_t)node_contents->dir.next_free);

                        // k_sys_TerminalPrintf("%d\n", child_node_index);
                        struct k_fs_pup_node_t *child_node = node_contents->dir.entries + child_node_index;

                        while(child_node_index != 0xffff)
                        {
                            struct k_fs_pup_node_t *child_node = node_contents->dir.entries + child_node_index;
                            // k_sys_TerminalPrintf("child node: %s %d %d\n", child_node->name, (uint32_t)child_node->left, (uint32_t)child_node->right);
                            if(child_node->type == K_FS_PUP_NODE_TYPE_LINK)
                            {

                            }
                            else
                            {
                                int32_t result = k_rt_StrCmp(child_node->name, path_fragment);

                                if(result == 0)
                                {
                                    k_fs_PupReleaseEntry(node_entry);
                                    next_node = child_node;
                                    next_node_link = K_FS_PUP_NODE_LINK(node_contents_block.link, child_node_index);
                                    node_entry = node_contents_entry;
                                    break;
                                }

                                if(result < 0)
                                {
                                    child_node_index = child_node->left;
                                }
                                else if(result > 0)
                                {
                                    child_node_index = child_node->right;
                                }

                                if(child_node_index == 0xffff)
                                {
                                    k_fs_PupReleaseEntry(node_contents_entry);
                                    break;
                                }

                                child_node = node_contents->dir.entries + child_node_index;
                            }
                        }            
                    }
                }
            }

            node = next_node;
            node_link = next_node_link;
        }

        k_fs_PupReleaseEntry(node_entry);
    }

    return node_link;
}

struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_centry_t *entry)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_node_t *node = NULL;
    struct k_fs_pup_link_t node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(node_address)};
    uint32_t node_index = K_FS_PUP_NODE_INDEX(node_address);

    uint32_t buffer_offset = (node_block.link - entry->first_block.link) * K_FS_PUP_LOGICAL_BLOCK_SIZE;
    union k_fs_pup_content_t *contents = (union k_fs_pup_content_t *)(entry->buffer + buffer_offset);

    if(node_index < 0xffff)
    {
        node = contents->dir.entries + node_index;

        if(node->type == K_FS_PUP_NODE_TYPE_NONE)
        {
            node = NULL;
        }
    }

    return node;
}

struct k_fs_pup_link_t k_fs_PupAddNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path, const char *name, uint32_t type)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_link_t parent = k_fs_PupFindNode(volume, path, start_node);
    struct k_fs_pup_link_t new_node_link = K_FS_PUP_NULL_LINK;

    if(parent.link != 0)
    {
        struct k_fs_pup_centry_t *parent_entry = k_fs_PupFindOrLoadCacheEntry(volume, (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(parent)});
        struct k_fs_pup_node_t *parent_node = k_fs_PupGetNode(volume, parent, parent_entry);
        uint32_t init_contents = 0;

        // k_sys_TerminalPrintf("parent entry goes from block %d to block %d\n", (uint32_t)parent_entry->first_block.link, (uint32_t)parent_entry->first_block.link + K_FS_PUP_BLOCKS_PER_ENTRY);

        if(!parent_node->contents.link)
        {
            parent_node->contents = k_fs_PupAllocBlock(volume);
            init_contents = 1;
        }


        struct k_fs_pup_centry_t *contents_entry = k_fs_PupFindOrLoadCacheEntry(volume, parent_node->contents);
        union k_fs_pup_content_t *contents = k_fs_PupGetBlock(volume, parent_node->contents, contents_entry);

        if(init_contents)
        {
            k_fs_PupInitContents(contents, parent, K_FS_PUP_CONTENT_TYPE_DIR);
        }

        uint16_t next_node_index = contents->dir.first_used;
        struct k_fs_pup_node_t *prev_node = NULL;
        int32_t compare_result = 0xffff;

        while(next_node_index != 0xffff)
        {
            prev_node = contents->dir.entries + next_node_index;

            if(prev_node->type == K_FS_PUP_NODE_TYPE_LINK)
            {
                struct k_fs_pup_link_t contents_link = prev_node->contents;
                k_fs_PupReleaseEntry(contents_entry);
                contents_entry = k_fs_PupFindOrLoadCacheEntry(volume, contents_link);
                contents = k_fs_PupGetBlock(volume, contents_link, contents_entry);
                next_node_index = contents->dir.first_used;
                prev_node = NULL;
                continue;
            }

            compare_result = k_rt_StrCmp(prev_node->name, name);

            if(compare_result < 0)
            {
                next_node_index = (uint16_t)prev_node->left;
            }
            else if(compare_result > 0)
            {
                next_node_index = (uint16_t)prev_node->right;
            }
            else
            {
                /* node already exists, so return a link to it */
                new_node_link = K_FS_PUP_NODE_LINK(parent_node->contents.link, next_node_index);
                break;
            }
        }

        if(compare_result != 0)
        {
            /* node doesn't exist, so create one */
            struct k_fs_pup_node_t *new_node = NULL;
            uint16_t node_index = contents->dir.next_free;
            new_node = contents->dir.entries + node_index;
            contents->dir.next_free = new_node->right;
            contents->dir.free_count--;
            k_rt_SetBytes(new_node, sizeof(struct k_fs_pup_node_t), 0);
            
            if(contents->dir.free_count == 0)
            {
                /* there's a single node left in this content buffer, so we'll allocate a new buffer, make it point to it,
                and then store our actual node in the new buffer */

                new_node->contents = k_fs_PupAllocBlock(volume);
                new_node->type = K_FS_PUP_NODE_TYPE_LINK;
                contents->dir.free_count--;
                struct k_fs_pup_centry_t *new_contents_entry = k_fs_PupFindOrLoadCacheEntry(volume, new_node->contents);
                union k_fs_pup_content_t *new_contents = k_fs_PupGetBlock(volume, new_node->contents, new_contents_entry);
                k_fs_PupInitContents(new_contents, K_FS_PUP_NODE_LINK(parent_node->contents.link, node_index), K_FS_PUP_CONTENT_TYPE_DIR);

                if(compare_result > 0)
                {
                    prev_node->right = node_index;
                }
                else if(compare_result < 0)
                {
                    prev_node->left = node_index;
                }
                
                /* there's no prev node in this new contents block */
                prev_node = NULL;

                contents_entry->flags |= K_FS_PUP_CENTRY_FLAG_DIRTY;
                k_fs_PupReleaseEntry(contents_entry);
                contents_entry = new_contents_entry;
                contents = new_contents;
                
                node_index = contents->dir.next_free;
                new_node = contents->dir.entries + node_index;
                contents->dir.next_free = new_node->right;
                k_rt_SetBytes(new_node, sizeof(struct k_fs_pup_node_t), 0);
            }

            new_node->type = type;
            new_node->left = 0xffff;
            new_node->right = 0xffff;

            k_rt_StrCpy(new_node->name, sizeof(new_node->name), name);
            if(prev_node == NULL)
            {
                /* no entries in this content buffer, so set this node as the first */
                contents->dir.first_used = node_index;
            }
            else
            {
                if(compare_result > 0)
                {
                    prev_node->right = node_index;
                }
                else if(compare_result < 0)
                {
                    prev_node->left = node_index;
                }
            }

            new_node_link = K_FS_PUP_NODE_LINK(parent_node->contents.link, node_index);
            contents_entry->flags |= K_FS_PUP_CENTRY_FLAG_DIRTY;
        }

        k_fs_PupReleaseEntry(contents_entry);
        k_fs_PupReleaseEntry(parent_entry);
    }

    return new_node_link;
}

void k_fs_PupRemoveNode(struct k_fs_vol_t *volume, const char *path)
{
    struct k_fs_pup_link_t node_link = k_fs_PupFindNode(volume, path, K_FS_PUP_NULL_LINK);

    if(node_link.link != K_FS_PUP_NULL_LINK.link)
    {
        struct k_fs_pup_link_t node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(node_link)};
        uint32_t node_index = K_FS_PUP_NODE_INDEX(node_link);
        struct k_fs_pup_centry_t *node_entry = k_fs_PupFindOrLoadCacheEntry(volume, node_block);
        union k_fs_pup_content_t *node_parent_contents = k_fs_PupGetBlock(volume, node_block, node_entry);
        struct k_fs_pup_node_t *node = node_parent_contents->dir.entries + node_index;

    }
}

void k_fs_PupInitContents(union k_fs_pup_content_t *contents, struct k_fs_pup_link_t node, uint32_t type)
{
    // contents->header.first_used = 0xffff;
    // contents->header.next_free = 0;

    uint16_t next_index = 0xffff;

    switch(type)
    {
        case K_FS_PUP_CONTENT_TYPE_DATA:
            contents->file.next_free = 0;
            contents->file.first_used = 0xffff;
            for(uint32_t index = K_FS_PUP_DATA_CONTENT_ENTRY_COUNT; index > 0;)
            {
                index--;
                contents->file.entries[index].contents = K_FS_PUP_NULL_LINK;
                contents->file.entries[index].right = next_index;
                contents->file.entries[index].offset = 0;
                next_index = index;
            }
        break;

        case K_FS_PUP_CONTENT_TYPE_DIR:
            contents->dir.parent = node;
            contents->dir.next_free = 0;
            contents->dir.first_used = 0xffff;
            contents->dir.free_count = K_FS_PUP_DIR_CONTENT_ENTRY_COUNT;
            for(uint32_t index = K_FS_PUP_DIR_CONTENT_ENTRY_COUNT; index > 0;)
            {
                index--;
                contents->dir.entries[index].contents = K_FS_PUP_NULL_LINK;
                contents->dir.entries[index].right = next_index;
                contents->dir.entries[index].type = K_FS_PUP_NODE_TYPE_NONE;
                next_index = index;
            }
        break;
    }
}

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node)
{
    // struct k_fs_pup_cset_t cache = {};
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_link_t node_link = k_fs_PupFindNode(volume, path, start_node);
    struct k_fs_pup_dirlist_t *dir_list = NULL;
    // uint32_t block_size = 1 << pup_volume->root.block_size_shift;

    if(node_link.link != K_FS_PUP_NULL_LINK.link)
    {
        
    }

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

void k_fs_PupFreeNodeDirList(struct k_fs_vol_t *volume, struct k_fs_pup_dirlist_t *dir_list)
{

}

uint32_t k_fs_PupGetPathToNode_r(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    // uint32_t block_size = 1 << pup_volume->root.block_size_shift;

    if(start_node.link == pup_volume->root.root_node.link)
    {
        path_buffer[0] = '\0';
        k_rt_StrCpy(path_buffer, buffer_size, "/");
        return buffer_size - 1;
    }
    else
    {
        // struct k_fs_pup_centry_t *node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(start_node), 1, NULL);
        struct k_fs_pup_link_t node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(start_node)};
        struct k_fs_pup_centry_t *node_entry = k_fs_PupFindOrLoadCacheEntry(volume, node_block);
        union k_fs_pup_content_t *contents = k_fs_PupGetBlock(volume, node_block, node_entry);
        struct k_fs_pup_node_t *node = contents->dir.entries + K_FS_PUP_NODE_INDEX(start_node);
        struct k_fs_pup_link_t parent_link = contents->dir.parent;
        uint32_t available_buffer_size = k_fs_PupGetPathToNode_r(volume, parent_link, path_buffer, buffer_size);
        uint32_t name_len = k_rt_StrLen(node->name) + 2;

        if(available_buffer_size >= name_len)
        {
            k_rt_StrCat(path_buffer, available_buffer_size, node->name);
            k_rt_StrCat(path_buffer, available_buffer_size, "/");
            available_buffer_size -= name_len;            
        }

        k_fs_PupReleaseEntry(node_entry);

        return available_buffer_size;
    }
}

void k_fs_PupGetPathToNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size)
{
    struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
    struct k_fs_pup_link_t node_block = (struct k_fs_pup_link_t){K_FS_PUP_NODE_BLOCK(start_node)};
    struct k_fs_pup_centry_t *node_entry = k_fs_PupFindOrLoadCacheEntry(volume, node_block);
    union k_fs_pup_content_t *contents = k_fs_PupGetBlock(volume, node_block, node_entry);
    struct k_fs_pup_node_t *node = contents->dir.entries + K_FS_PUP_NODE_INDEX(start_node);

    if(node->type != K_FS_PUP_NODE_TYPE_NONE)
    {
        k_fs_PupGetPathToNode_r(volume, start_node, path_buffer, buffer_size);
    } 

    k_fs_PupReleaseEntry(node_entry);
}

// void k_fs_PupFlushCache(struct k_fs_vol_t *volume)
// {

// }

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
}