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

struct k_fs_pup_root_t
{
    char ident[K_FS_PUP_IDENT];
    struct k_fs_pup_link_t root_node;
    uint16_t bitmask_block_start;
    uint16_t bitmask_block_count;
    uint16_t available_block_start;
    
    
    /* block size, in bytes */
    uint16_t block_size;
    uint8_t node_index_shift;
    /* node index of the file system root node */
    // uint8_t root_node;
};

struct k_fs_pup_range_t
{
    uint64_t first_block : 40;
    uint64_t block_count : 24;
};

/* directory entry */
struct k_fs_pup_dirent_t
{
    struct k_fs_pup_link_t node;
    char name[64 - sizeof(struct k_fs_pup_link_t)];
};

struct k_fs_pup_dirlist_t
{
    uint32_t used_count;
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
    uint8_t type;
    uint8_t flags;
    uint16_t align;
    k_rt_spnl_t lock;
    
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

enum K_FS_PUP_SLOT_FLAGS
{
    K_FS_PUP_SLOT_FLAG_DIRTY = 1,
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
    // k_rt_spnl_t read_lock;
    uint32_t read_count;
    k_rt_spnl_t write_lock;
};

struct k_fs_pup_volume_t
{
    struct k_fs_pup_root_t    root;
    // int32_t                  block_size_shift;
    // uint32_t                 cached_block_buffer_size;
    // uint8_t                 *cached_blocks_base;
    // uint32_t                  used_slots;
    uint32_t                  lru_bitmask;
    uint32_t                  allocated_memory;
    struct k_fs_pup_cset_t    cache_sets[K_FS_PUP_CACHE_SET_COUNT];
    // struct k_fs_pup_centry_t *entry_pool;
};

void k_fs_PupMountVolume(struct k_fs_vol_t *volume);

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume);

uint32_t k_fs_PupTryCopyEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry, uint8_t *block_buffer, uint32_t block_start, uint32_t block_count);

void k_fs_PupTouchEntry(struct k_fs_pup_volume_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry);

void k_fs_PupRead(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer);

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer);

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_vol_t *volume);

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry);

void k_fs_PupCacheEntry(struct k_fs_pup_volume_t *volume, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupEvictOldestEntry(struct k_fs_vol_t *volume);

// struct k_fs_pup_slot_t *k_fs_PupFindSlot(struct k_fS_vol_t *volume, uint32_t block_address);

void k_fs_PupFlushCache(struct k_fs_vol_t *volume);

#endif
