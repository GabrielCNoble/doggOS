#ifndef K_FS_PUP_H
#define K_FS_PUP_H

#include "defs.h"
#include "../rt/atm.h"

#define K_FS_PUP_LOGICAL_BLOCK_SIZE 0x200
#define K_FS_PUP_MAX_RNODE_RANGES 4
#define K_FS_PUP_MAX_LNODE_LINKS 4

#define K_FS_PUP_IDENT 24
#define K_FS_PUP_MAGIC "OWO UWU LE PUP FS"

#define K_FS_PUP_FREE_BITMASK_BLOCK_BITS 2
#define K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE (8 / K_FS_PUP_FREE_BITMASK_BLOCK_BITS)

enum K_FS_PUP_BLOCK_STATUS
{
    /* free block */
    K_FS_PUP_BLOCK_STATUS_FREE = 0,
    /* block allocated to file or dir data */
    K_FS_PUP_BLOCK_STATUS_DATA,
    /* block allocated to nodes, with space available */
    K_FS_PUP_BLOCK_STATUS_NODE,
    /* block allocated to nodes, without space available */
    K_FS_PUP_BLOCK_STATUS_FULL,
};

struct k_fs_pup_link_t
{
    uint64_t link;
};

#define K_FS_PUP_NULL_LINK ((struct k_fs_pup_link_t){0})

struct k_fs_pup_root_t
{
    char ident[K_FS_PUP_IDENT];
    struct k_fs_pup_link_t root_node;
    uint16_t bitmask_block_start;
    uint16_t bitmask_block_count;
    uint16_t available_block_start;
    
    
    /* block size, in bytes */
    uint16_t block_size;
    /* node index of the file system root node */
    uint8_t node_index_shift;
    // uint8_t root_node;
};

#define K_FS_PUP_RANGE_FIRST_BLOCK_BITS 40
#define K_FS_PUP_RANGE_BLOCK_COUNT_BITS 24

struct k_fs_pup_range_t
{
    // uint64_t first_block : 40;
    // uint64_t block_count : 24;
    uint64_t first_count;
};

#define K_FS_PUP_RANGE_FIRST_BLOCK(first_count) ((first_count) >> K_FS_PUP_RANGE_FIRST_BLOCK_BITS);
#define K_FS_PUP_RANGE_BLOCK_COUNT(first_count) ((first_count) & ((1 << K_FS_PUP_RANGE_BLOCK_COUNT_BITS) - 1))
#define K_FS_PUP_RANGE_FIRST_COUNT(first_block, block_count) ( (((uint64_t)(first_block)) << K_FS_PUP_RANGE_FIRST_BLOCK_BITS) | ((uint64_t)(block_count)) )

/* directory entry */
struct k_fs_pup_dirent_t
{
    struct k_fs_pup_link_t node;
    char name[64 - sizeof(struct k_fs_pup_link_t)];
};

struct k_fs_pup_dirlist_t
{
    uint32_t used_count;
    uint32_t align0;
    struct k_fs_pup_dirent_t entries[];
};

enum K_FS_PUP_NODE_TYPE
{
    K_FS_PUP_NODE_TYPE_NONE = 0,
    /* first node in the list of nodes */
    K_FS_PUP_NODE_TYPE_ROOT,
    /* node contains a list of block ranges, containing file data */
    K_FS_PUP_NODE_TYPE_FILE,
    /* node contains a list of links to other nodes */
    K_FS_PUP_NODE_TYPE_LINK,
    /* node contains a list of block ranges, containing directory entries */
    K_FS_PUP_NODE_TYPE_DIR,
};

struct k_fs_pup_node_t
{    
    // uint8_t type;
    // uint8_t flags;
    // uint16_t align;
    uint32_t type;
    uint32_t flags;
    uint32_t align;
    uint32_t lock;
    // k_rt_spnl_t lock;
    struct k_fs_pup_link_t parent;
    
    union
    {
        struct k_fs_pup_range_t ranges[K_FS_PUP_MAX_RNODE_RANGES];
        struct k_fs_pup_link_t links[2];
    };
};

#define K_FS_PUP_CACHE_SET_COUNT 32
#define K_FS_PUP_BLOCKS_PER_ENTRY 4

#define K_FS_PUP_CACHE_SET_INDEX_SHIFT 2
#define K_FS_PUP_CACHE_SET_INDEX_MASK (K_FS_PUP_CACHE_SET_COUNT - 1)
#define K_FS_PUP_CACHE_SET_INDEX(block_address) (((block_address) >> K_FS_PUP_CACHE_SET_INDEX_SHIFT) & K_FS_PUP_CACHE_SET_INDEX_MASK)

/* max of 16 MB of memory dedicated for a single disk */
#define K_FS_PUP_MAX_CACHE_MEM 0x1000000

enum K_FS_PUP_CENTRY_FLAGS
{
    K_FS_PUP_CENTRY_FLAG_DIRTY = 1,
};

struct k_fs_pup_centry_t
{
    struct k_fs_pup_centry_t *next;
    struct k_fs_pup_centry_t *prev;
    k_rt_spnl_t               touch_lock;
    uint32_t                  first_block;
    uint16_t                  flags;
    uint16_t                  align0;
    uint8_t                   buffer[];
};

struct k_fs_pup_cset_t
{
    struct k_fs_pup_centry_t *first_entry;
    struct k_fs_pup_centry_t *last_entry;
    uint32_t read_count;
    k_rt_spnl_t write_lock;
};

struct k_fs_pup_centry_copy_t
{
    uint32_t entry_buffer_offset;
    uint32_t data_buffer_offset;
    uint32_t copy_size;
};

struct k_fs_pup_volume_t
{
    struct k_fs_pup_root_t    root;
    uint32_t                  lru_bitmask;
    uint32_t                  allocated_memory;
    struct k_fs_pup_cset_t    cache_sets[K_FS_PUP_CACHE_SET_COUNT];
    struct k_fs_pup_centry_t *entry_pool;
};

enum K_FS_PUP_PATH_TYPE
{
    K_FS_PUP_PATH_TYPE_CUR_DIR,
    K_FS_PUP_PATH_TYPE_PARENT_DIR,
    K_FS_PUP_PATH_TYPE_ROOT,
    K_FS_PUP_PATH_TYPE_NORMAL,
};

struct k_fs_pup_path_t
{
    const char *path_buffer;
    uint32_t start;
    uint32_t end;
};

void k_fs_PupMountVolume(struct k_fs_vol_t *volume);

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/


void k_fs_PupTouchEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry);

void k_fs_PupCacheEntry(struct k_fs_pup_volume_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSetWithCopyFields(struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end, struct k_fs_pup_centry_copy_t *copy);

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSet(struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end);

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_vol_t *volume);

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupDropOldestEntry(struct k_fs_vol_t *volume);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupRead(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer);

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupNextPathComponent(struct k_fs_pup_path_t *path);

struct k_fs_pup_centry_t *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint64_t block_address);

// void k_fs_PupReleaseBlock(struct k_fs_vol_t *volume, void *block);

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node, struct k_fs_pup_cset_t *cache);

struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_cset_t *cache);

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/


// struct k_fs_pup_slot_t *k_fs_PupFindSlot(struct k_fS_vol_t *volume, uint32_t block_address);

void k_fs_PupFlushCache(struct k_fs_vol_t *volume);

#endif
