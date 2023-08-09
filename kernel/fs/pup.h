#ifndef K_FS_PUP_H
#define K_FS_PUP_H

/*
    TODO:
        
        - to avoid having a thread (or many threads) block while waiting for 
        an entry from disk, split a read operation in cache entries (32 entries,
        for example), and store the result in a 32 bitmask, with each bit set
        representing a entry it needs. In this scheme, bit 0 means the first
        entry it needs to read, bit 1 represents the second, and the nth-1 bit 
        set is the last entry it needs to read. This way, a thread can proactively
        copy entries from sets that aren't locked while it awaits for the non-available
        entries to arrive. Whenever it reads one entry, it resets the bit in the bitmask.

        - entries in a set should be kept sorted, so something like a binary
        search can be used to find if an entry is already present.
*/

#include "defs.h"
#include "../rt/atm.h"
#include "../rt/pool.h"

/* 4096 fixed size block */
#define K_FS_PUP_LOGICAL_BLOCK_SIZE 0x1000
#define K_FS_PUP_MAX_RNODE_RANGES 4
#define K_FS_PUP_MAX_LNODE_LINKS 4

#define K_FS_PUP_IDENT 24
#define K_FS_PUP_MAGIC "OWO UWU LE PUP FS"

#define K_FS_PUP_FREE_BITMASK_BLOCK_BITS 2
#define K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE (8 / K_FS_PUP_FREE_BITMASK_BLOCK_BITS)

/* purpose of a block. This file system doesn't keep separate 
bitmaps for blocks used to store data and blocks used for nodes. */

/* TODO: rename this to something better!!!!! */
// enum K_FS_PUP_BLOCK_TYPE 
// {
//     /* free block */
//     K_FS_PUP_BLOCK_TYPE_FREE = 0,
//     /* block allocated to file or dir data */
//     K_FS_PUP_BLOCK_TYPE_CONTENT,
//     /* block allocated to nodes, with space available */
//     K_FS_PUP_BLOCK_TYPE_NODE,
//     /* block allocated to nodes, without space available */
//     K_FS_PUP_BLOCK_TYPE_FULL,
// };

struct k_fs_pup_link_t
{
    uint64_t link;
};



#define K_FS_PUP_NULL_LINK ((struct k_fs_pup_link_t){0})


// #define K_FS_PUP_RANGE_START_BITS 40
// #define K_FS_PUP_RANGE_COUNT_BITS 24

// struct k_fs_pup_range_t
// {
//     uint64_t contents;
// };

// #define K_FS_PUP_RANGE_START(first_count) ((first_count) >> K_FS_PUP_RANGE_START_BITS);
// #define K_FS_PUP_RANGE_COUNT(first_count) ((first_count) & ((1 << K_FS_PUP_RANGE_COUNT_BITS) - 1))
// #define K_FS_PUP_RANGE_MAKE_RANGE(first_block, block_count) ((struct k_fs_pup_range_t){ ((((uint64_t)(first_block)) << K_FS_PUP_RANGE_START_BITS) | ((uint64_t)(block_count))) })


/* directory entry. Each entry reserves 248 bytes for node names, which
boils down to 62 utf-8 encoded chars. Each such entry can either point at a
node or at another content block containing more dir entries. */
// struct k_fs_pup_dirent_t
// {
//     /* this will either point at a content block or a node */
//     struct k_fs_pup_link_t      link;

//     /* those entries form a binary tree inside a content block, so those "pointers"
//     are indexes from the start of the content block that contains this dir entry. */
//     uint8_t                     left;
//     uint8_t                     right;

//     char                        name[256 - 10];
// };

/* file data entry. Each such entry can either point at a block containing
raw file data or at another content block containing more file entries inside. */
struct k_fs_pup_datent_t
{
    /* this will either point at a content block or a data block */
    struct k_fs_pup_link_t      contents;
    /* offset into the file */
    uint64_t                    offset;
    /* those entries form a binary tree inside a content block, so those "pointers"
    are indexes from the start of the content block that contains this data entry. */
    uint8_t                     left;
    uint8_t                     right;
};

enum K_FS_PUP_CONTENT_TYPE
{
    K_FS_PUP_CONTENT_TYPE_DATA = 0,
    K_FS_PUP_CONTENT_TYPE_DIR,
};

enum K_FS_PUP_NODE_TYPE
{
    K_FS_PUP_NODE_TYPE_NONE = 0,    
    /* node represents a file */
    K_FS_PUP_NODE_TYPE_FILE,
    /* node represents a directory */
    K_FS_PUP_NODE_TYPE_DIR,
    /* node links content blocks */
    K_FS_PUP_NODE_TYPE_LINK,
};

// enum K_FS_PUP_NODE_CONTENTS
// {
//     /* node contains nothing */
//     K_FS_PUP_NODE_CONTENT_NONE = 0,
//     /* node contains a list of block ranges */
//     // K_FS_PUP_NODE_CONTENT_DATA,
//     /* node contains links to other nodes */
//     // K_FS_PUP_NODE_CONTENT_LINK,
// };

enum K_FS_PUP_NODE_FLAGS
{
    K_FS_PUP_NODE_FLAG_INFO = 1,
};

#define K_FS_PUP_NODE_INDEX_BITS 8
#define K_FS_PUP_NODE_LINK(block_index, node_index) ((struct k_fs_pup_link_t){ ((block_index) << K_FS_PUP_NODE_INDEX_BITS) | \
                                                        ((node_index) & ((1 << (K_FS_PUP_NODE_INDEX_BITS - 1)) - 1)) })

#define K_FS_PUP_NODE_BLOCK(pup_link) ((pup_link.link) >> K_FS_PUP_NODE_INDEX_BITS)
#define K_FS_PUP_NODE_INDEX(pup_link) ((pup_link.link) & ((1 << K_FS_PUP_NODE_INDEX_BITS) - 1))

#define K_FS_PUP_NODE_FIELDS                            \
    struct k_fs_pup_link_t              contents;       \
    union                                               \
    {                                                   \
        uint64_t                        size;           \
        struct k_fs_pup_node_info_t *   info;           \
    };                                                  \
                                                        \
    union                                               \
    {                                                   \
        k_rt_spnl_t                     info_lock;      \
        uint32_t                        unused0;        \
    };                                                  \
                                                        \
    uint8_t                     left;                   \
    uint8_t                     right;                  \
    uint8_t                     type;                   \
    uint8_t                     flags;                  \

#define K_FS_PUP_NODE_NAME_MAX_LEN (256 - sizeof(struct {K_FS_PUP_NODE_FIELDS;}))

struct k_fs_pup_node_t
{    
    K_FS_PUP_NODE_FIELDS;
    uint8_t     name[K_FS_PUP_NODE_NAME_MAX_LEN];
};

struct k_fs_pup_node_info_t
{
    K_RT_POOL_NODE;
    struct k_fs_pup_link_t      node;
    uint64_t                    size;
    uint64_t                    ref_count;
    struct k_fs_pup_centry_t *  node_entry;
    k_rt_spnl_t                 write_lock;
};

struct k_fs_pup_node_list_item_t
{
    K_RT_POOL_NODE;
    struct k_fs_pup_node_list_item_t *  next;
    struct k_fs_pup_node_t *            node;
};

struct k_fs_pup_node_list_t
{
    struct k_rt_pool_t                  entry_pool;
    struct k_fs_pup_node_list_item_t *  nodes;
};
/* forward declaration */
// struct k_fs_pup_node_info_page_t;

// struct k_fs_pup_node_info_t
// {
//     struct k_fs_pup_node_info_t *       next;
//     struct k_fs_pup_node_info_t *       prev;
//     struct k_fs_pup_node_info_page_t *  page;
//     struct k_fs_pup_link_t              node;
//     uint64_t                            size;
//     uint32_t                            read_count;
// };

// #define K_FS_PUP_NODE_INFO_PAGE_FIELDS                 \
//     struct k_fs_pup_node_info_page_t * next_page;      \
//     struct k_fs_pup_node_info_page_t * prev_page;      \
//     struct k_fs_pup_node_info_t *      first_free;     \
//     uint32_t                           index;          \
//     uint32_t                           used_count;     \

// #define K_FS_PUP_NODE_INFO_PAGE_SIZE 4096
// #define K_FS_PUP_NODE_INFO_PAGE_ENTRY_COUNT ((K_FS_PUP_NODE_INFO_PAGE_SIZE - sizeof(struct {K_FS_PUP_NODE_INFO_PAGE_FIELDS;})) / sizeof(struct k_fs_pup_node_info_t))

// struct k_fs_pup_node_info_page_t
// {
//     K_FS_PUP_NODE_INFO_PAGE_FIELDS;
//     struct k_fs_pup_node_info_t entries[K_FS_PUP_NODE_INFO_PAGE_ENTRY_COUNT];
// };

/* node content block. This contains directory entries, in case of a directory node, or
data entries, in case of a file node. */

// struct k_fs_pup_content_header_t
// {
//     uint16_t first_used;
//     uint16_t next_free;
//     uint16_t pad[6];
// };

// #define K_FS_PUP_CONTENT_DATA_SIZE (K_FS_PUP_LOGICAL_BLOCK_SIZE - sizeof(struct k_fs_pup_content_header_t))


#define K_FS_PUP_DIR_CONTENT_FIELDS         \
    struct k_fs_pup_link_t  parent;         \
    uint8_t                 first_used;     \
    uint8_t                 next_free;      \
    uint16_t                free_count;     \
    uint16_t                pad[2];         \

#define K_FS_PUP_DIR_CONTENT_ENTRY_COUNT    \
    (K_FS_PUP_LOGICAL_BLOCK_SIZE / ((K_FS_PUP_LOGICAL_BLOCK_SIZE - sizeof(struct {K_FS_PUP_DIR_CONTENT_FIELDS;})) / sizeof(struct k_fs_pup_node_t)))

#define K_FS_PUP_DATA_CONTENT_FIELDS        \
    uint16_t                first_used;     \
    uint16_t                next_free;      \
    uint16_t                pad[2];         \

#define K_FS_PUP_DATA_CONTENT_ENTRY_COUNT   \
    (K_FS_PUP_LOGICAL_BLOCK_SIZE / ((K_FS_PUP_LOGICAL_BLOCK_SIZE - sizeof(struct {K_FS_PUP_DATA_CONTENT_FIELDS;})) / sizeof(struct k_fs_pup_datent_t)))
 
union k_fs_pup_content_t
{
    struct
    {
        /* child nodes of a node */
        K_FS_PUP_DIR_CONTENT_FIELDS;
        struct k_fs_pup_node_t entries[K_FS_PUP_DIR_CONTENT_ENTRY_COUNT];
    }dir;

    struct
    {
        /* file data if file size > 4KB */
        K_FS_PUP_DATA_CONTENT_FIELDS;
        struct k_fs_pup_datent_t entries[K_FS_PUP_DATA_CONTENT_ENTRY_COUNT];
    }file;

    /* file data if file size <= 4KB */
    uint8_t data[K_FS_PUP_LOGICAL_BLOCK_SIZE];
};

// struct k_fs_pup_lcontent_t
// {
//     struct k_fs_pup_content_t *contents;
// };

struct k_fs_pup_dirent_t
{
    struct k_fs_pup_link_t  node;
    uint8_t                 name[K_FS_PUP_NODE_NAME_MAX_LEN];
};

struct k_fs_pup_dirlist_t
{
    struct k_fs_pup_dirlist_t *     next;
    uint32_t                        entry_count;
    struct k_fs_pup_dirent_t        entries[K_FS_PUP_DIR_CONTENT_ENTRY_COUNT];
};


/* MUST be a power of 2 */
#define K_FS_PUP_CACHE_SET_COUNT 32
/* MUST be a power of 2 */
#define K_FS_PUP_BLOCKS_PER_ENTRY 4

/* MUST be log2(K_FS_PUP_BLOCKS_PER_ENTRY) */
#define K_FS_PUP_CACHE_SET_INDEX_SHIFT 2
#define K_FS_PUP_CACHE_SET_INDEX_MASK (K_FS_PUP_CACHE_SET_COUNT - 1)
#define K_FS_PUP_CACHE_SET_INDEX(block_address) (((block_address) >> K_FS_PUP_CACHE_SET_INDEX_SHIFT) & K_FS_PUP_CACHE_SET_INDEX_MASK)

/* max of 16 MB of memory dedicated for a single disk */
#define K_FS_PUP_MAX_CACHE_MEM 0x1000000

enum K_FS_PUP_CENTRY_FLAGS
{
    /* modified, not written back yet */
    K_FS_PUP_CENTRY_FLAG_DIRTY = 1,
    /* allocated but waiting for contents to arrive */
    K_FS_PUP_CENTRY_FLAG_PENDING = 1 << 1,
    K_FS_PUP_CENTRY_FLAG_READY = 1 << 2,
};

struct k_fs_pup_centry_t
{
    struct k_fs_pup_centry_t *          next;
    struct k_fs_pup_centry_t *          prev;
    struct k_fs_pup_centry_page_t *     page;
    
    uint32_t                            ref_count;
    k_rt_spnl_t                         lock;
    k_rt_spnl_t                         init_lock;
    k_rt_cond_t                         condition;
    uint32_t                            flags;
    struct k_fs_pup_link_t              first_block; 
    uint8_t                             buffer[K_FS_PUP_LOGICAL_BLOCK_SIZE * K_FS_PUP_BLOCKS_PER_ENTRY];
};

/* 8MB per centry page */
#define K_FS_PUP_CENTRY_PAGE_SIZE 0x800000

#define K_FS_PUP_CENTRY_PAGE_FIELDS                 \
    struct k_fs_pup_centry_page_t * next;           \
    struct k_fs_pup_centry_page_t * prev;           \
    struct k_fs_pup_centry_t *      free_entries;   \
    uint32_t                        used_entries;   \
    uint32_t                        index;          \

#define K_FS_PUP_CENTRY_PAGE_ENTRIES_COUNT          \
    ((K_FS_PUP_CENTRY_PAGE_SIZE - sizeof(struct {K_FS_PUP_CENTRY_PAGE_FIELDS;})) / sizeof(struct k_fs_pup_centry_t))

struct k_fs_pup_centry_page_t
{
    K_FS_PUP_CENTRY_PAGE_FIELDS;
    struct k_fs_pup_centry_t entries[K_FS_PUP_CENTRY_PAGE_ENTRIES_COUNT];
};


/* FIXME: cache sets can be local variables, which serve to keep entries alive while code is 
accessing it without needing any sort of synchronization. While this is good for performance
there's the possibility of an entry in a local cache set to become stale compared to the file
system cache (since the local cache set is holding a copy of the blocks), which could cause all 
sorts of funny issues. This needs to be addressed! */
struct k_fs_pup_cset_t
{
    struct k_fs_pup_centry_t *              first_entry;
    struct k_fs_pup_centry_t *              last_entry;
    /* how many threads are currently touching this line. Necessary to keep the
    cache consistent for traversals */
    uint32_t                                read_count;
    /* only one thread at time can modify the cache */
    k_rt_spnl_t                             lock;
    // k_rt_spnl_t                 insert_lock;
};

struct k_fs_pup_cbuffer_t
{
    struct k_fs_pup_centry_t *      entry;
    void *                          buffer;
};

struct k_fs_pup_centry_copy_t
{
    uint32_t entry_buffer_offset;
    uint32_t data_buffer_offset;
    uint32_t copy_size;
};


struct k_fs_pup_root_t
{
    char                        ident[K_FS_PUP_IDENT];
    struct k_fs_pup_link_t      root_node;
    // struct k_fs_pup_node_t      root_node;
    struct k_fs_pup_link_t      bitmask_start;
    uint64_t                    bitmask_block_count;

    /* first allocatable block after the last bitmask block */
    struct k_fs_pup_link_t      alloc_start;
    /* number of blocks in the volume */
    uint64_t                    block_count;    
};

struct k_fs_pup_vol_t
{
    struct k_fs_pup_root_t                  root;
    /* LRU bitmask, used for finding which set has been touched last. The
    last entry in a set's entry linked list is the least recently accessed entry */
    uint32_t                                lru_bitmask;


    // k_rt_spnl_t                             node_lock_info_lock;

    /* how much virtual memory is dedicated to this cache. It'll keep growing
    until hitting the max memory allowed for a disk cache. No entry evicion will
    happen as long as the cache is smaller than this. */
    // uint32_t                        allocated_memory;

    /* disk cache. It's a N-way set-associative cache. The number of ways 
    per set is variable. */
    struct k_fs_pup_cset_t                  cache_sets[K_FS_PUP_CACHE_SET_COUNT];

    struct k_fs_pup_centry_page_t *         entry_pages;
    struct k_fs_pup_centry_page_t *         last_entry_page;
    struct k_fs_pup_centry_page_t *         cur_entry_page;
    struct k_fs_pup_centry_page_t *         freeable_entry_pages;
    k_rt_spnl_t                             entry_page_lock;
    uint32_t                                entry_page_count;

    struct k_rt_pool_t                      node_info;
    k_rt_spnl_t                             node_info_lock;



    k_rt_spnl_t                             lock;

    /* in-memory block bitmask, to avoid touching the disk when allocating
    blocks. Each byte keeps allocation state for 4 consecutive blocks */

    /* TODO: a more scalable solution is needed. For small volumes this is
    fine, specially if the block size is relatively large. But for larger 
    volumes this will take too much memory. For a 2TB volume, for example,
    this will take around 128MB, which is not great. 
    
    Also, this is pretty slow since it requires a linear scan over an array
    of bytes, and some additional bit twiddling per byte. Something similar
    to the kernel allocator could be used for this when the volume is mounted,
    and the bitmask representation would be used only on disk. */
    uint8_t *                               block_bitmask;
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
    const char *    path_buffer;
    uint32_t        start;
    uint32_t        end;
};

struct k_fs_pup_format_args_t
{
    uint32_t block_size;
    uint32_t block_count;
};

void k_fs_PupMountVolume(struct k_fs_vol_t *volume);

uint32_t k_fs_PupFormatVolume(struct k_fs_vol_t *volume, void *args);

void k_fs_PupUnmountVolume(struct k_fs_vol_t *volume);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

void k_fs_PupTouchEntry(struct k_fs_pup_vol_t *volume, uint32_t set_index, struct k_fs_pup_centry_t *entry);

// uint32_t k_fs_PupCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupCacheEntryAddress(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t address);

// struct k_fs_pup_centry_t *k_fs_PupFindCacheEntryWithCopyFields(struct k_fs_vol_t *volume, struct k_fs_pup_link_t first_block, uint32_t block_count, struct k_fs_pup_centry_copy_t *copy);

struct k_fs_pup_centry_t *k_fs_PupFindCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block);

struct k_fs_pup_centry_t *k_fs_PupFindOrLoadCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block);

void k_fs_PupStoreCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry);

void k_fs_PupStashCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_t *entry);

void k_fs_PupAcquireEntry(struct k_fs_pup_centry_t *entry);

void k_fs_PupReleaseEntry(struct k_fs_pup_centry_t *entry);

void k_fs_PupInitCacheEntryPage(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_page_t *page);

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_pup_vol_t *volume);

// struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntryForAddress(struct k_fs_vol_t *volume, uint32_t address);

void k_fs_PupFreeCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_centry_t *entry);

void k_fs_PupFlushCache(struct k_fs_vol_t *volume);

// struct k_fs_pup_centry_t *k_fs_PupDropOldestEntry(struct k_fs_pup_vol_t *volume);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

// struct k_fs_pup_centry_t *k_fs_PupLoadEntry(struct k_fs_vol_t *volume, uint64_t block_address, uint64_t block_count, struct k_fs_pup_centry_copy_t *copy);

// void k_fs_PupReadVolume(struct k_fs_vol_t *volume, uint64_t first_block, uint32_t block_count, void *buffer);

void k_fs_PupRead(struct k_fs_vol_t *volume, uint64_t first_block, uint32_t block_count, void *buffer);

void k_fs_PupWrite(struct k_fs_vol_t *volume, uint32_t block_start, uint32_t block_count, void *buffer);

// void k_fs_PupFlushDirtyEntries(struct k_fs_vol_t *volume);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

// void k_fs_PupNextPathComponent(struct k_fs_pup_path_t *path);

// void *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block);

// struct k_fs_pup_range_t k_fs_PupAllocRange(struct k_fs_vol_t *volume, uint32_t count, uint32_t alloc_type);

// uint32_t k_fs_PupTryReallocRange(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t count, uint32_t alloc_type);

// void k_fs_PupSetRangeType(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t type);

// void k_fs_PupFreeRange(struct k_fs_pup_range_t *range);

void *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_link_t k_fs_PupAllocBlock(struct k_fs_vol_t *volume);

void k_fs_PupFreeBlock(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block);

// uint32_t k_fs_PupChangeBlockBit(struct k_fs_vol_t *volume, struct k_fs_pup_link_t block, uint32_t set);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

// struct k_fs_pup_link_t k_fs_PupAllocNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t parent_node);

// void k_fs_PupFreeNode(struct k_fs_pup_link_t link);

struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address);

struct k_fs_pup_node_t *k_fs_PupGetNodeOnEntry(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_centry_t *entry);

// struct k_fs_pup_node_t *k_fs_PupGetNodeOnContents(struct k_fs_vol_t *volume, uint32_t index, union k_fs_pup_content_t *contents);

// void k_fs_PupAcquireNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address);

// void k_fs_PupAcquireNode(struct k_fs_vol_t *volume, struct k_fs_pup_node_t *node);

void k_fs_PupReleaseNode(struct k_fs_vol_t *volume, struct k_fs_pup_node_t *node);

void k_fs_PupLockNode(struct k_fs_vol_t *volume, struct k_fs_pup_node_t *node);

void k_fs_PupUnlockNode(struct k_fs_vol_t *volume, struct k_fs_pup_node_t *node);

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node);

struct k_fs_pup_link_t k_fs_PupAddNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path, const char *name, uint32_t type);

void k_fs_PupRemoveNode(struct k_fs_vol_t *volume, const char *path);

void k_fs_PupInitContents(union k_fs_pup_content_t *contents, struct k_fs_pup_link_t node, uint32_t type);

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node);

void k_fs_PupFreeNodeDirList(struct k_fs_vol_t *volume, struct k_fs_pup_dirlist_t *dir_list);

void k_fs_PupGetPathToNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_file_t *k_fs_PupOpenFile(struct k_fs_vol_t *volume, const char *path, const char *mode);

void k_fs_PupCloseFile(struct k_fs_vol_t *volume, struct k_fs_file_t *file);

struct k_fs_pup_link_t k_fs_PupOpenFileNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path);

void k_fs_PupCloseFileNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t file_node);

void k_fs_PupWriteFileData(struct k_fs_vol_t *volume, struct k_fs_pup_link_t file_node, uint64_t offset, uint64_t size, void *data);

void k_fs_PupReadFileData(struct k_fs_vol_t *volume, struct k_fs_pup_link_t file_node, uint64_t offset, uint64_t size, void *data);


// struct k_fs_pup_slot_t *k_fs_PupFindSlot(struct k_fS_vol_t *volume, uint32_t block_address);

// void k_fs_PupFlushCache(struct k_fs_vol_t *volume);

#endif
