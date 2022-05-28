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

// #define K_FS_PUP_LOGICAL_BLOCK_SIZE 0x200
#define K_FS_PUP_MAX_RNODE_RANGES 4
#define K_FS_PUP_MAX_LNODE_LINKS 4

#define K_FS_PUP_IDENT 24
#define K_FS_PUP_MAGIC "OWO UWU LE PUP FS"

#define K_FS_PUP_FREE_BITMASK_BLOCK_BITS 2
#define K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE (8 / K_FS_PUP_FREE_BITMASK_BLOCK_BITS)

/* purpose of a block. This file system doesn't keep separate 
bitmaps for blocks used to store data and blocks used for nodes. */

/* TODO: rename this to something better!!!!! */
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
#define K_FS_PUP_MAKE_LINK(block_index, node_index, node_bits) ((struct k_fs_pup_link_t){ ((block_index) << node_bits) | ((node_index) & ((1 << (node_bits)) - 1)) })
#define K_FS_PUP_NODE_BLOCK(link, index_bits) ((link.link) >> (index_bits))
#define K_FS_PUP_NODE_INDEX(link, index_bits) ((link.link) & ((1 << (index_bits)) - 1))

struct k_fs_pup_root_t
{
    char                        ident[K_FS_PUP_IDENT];
    /* generally the first node in the first allocatable block, but can be
    allocated anywhere in the disk */
    struct k_fs_pup_link_t      root_node;
    /* first block used for the block bitmask. Currently this is always 1 */
    uint8_t                     bitmask_start;
    /* block size shift, size in bytes is 1 << block_size_shift. 1 << 255 is
    a pretty obscene limit. */
    uint8_t                     block_size_shift;
    /* number of bits in a link dedicated to a node index. A node address
    is composed of a block index and a node index, stuffed in a uint64_t.
    The node index part of the address is node_index_bits wide, and is
    stored in the least significant bits.  */
    uint8_t                     node_index_bits;
    uint8_t                     align0;
    uint32_t                    align1;

    /* first allocable block after the last bitmask block */
    uint64_t                    alloc_start;
    /* number of blocks in the volume */
    uint64_t                    block_count;    
};

#define K_FS_PUP_RANGE_START_BITS 40
#define K_FS_PUP_RANGE_COUNT_BITS 24

struct k_fs_pup_range_t
{
    // uint64_t contents;
    uint64_t start;
    uint64_t count;
};

// #define K_FS_PUP_RANGE_START(first_count) ((first_count) >> K_FS_PUP_RANGE_START_BITS);
// #define K_FS_PUP_RANGE_COUNT(first_count) ((first_count) & ((1 << K_FS_PUP_RANGE_COUNT_BITS) - 1))
// #define K_FS_PUP_RANGE_MAKE_RANGE(first_block, block_count) ((struct k_fs_pup_range_t){ ((((uint64_t)(first_block)) << K_FS_PUP_RANGE_START_BITS) | ((uint64_t)(block_count))) })


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
    /* node contains a list of block ranges, containing file data */
    K_FS_PUP_NODE_TYPE_FILE,
    /* node contains a list of links to other nodes */
    K_FS_PUP_NODE_TYPE_LINK,
    /* node contains a list of block ranges, containing directory entries */
    K_FS_PUP_NODE_TYPE_DIR,
};

enum K_FS_PUP_NODE_CONTENTS
{
    /* node contains nothing */
    K_FS_PUP_NODE_CONTENT_NONE,
    /* node contains a list of block ranges */
    K_FS_PUP_NODE_CONTENT_DATA,
    /* node contains links to other nodes */
    K_FS_PUP_NODE_CONTENT_LINK,
};

struct k_fs_pup_node_t
{    
    uint8_t                     type;
    uint8_t                     flags;
    uint8_t                     content_type;
    uint8_t                     align0;
    uint32_t                    child_count;
    uint32_t                    align1;
    k_rt_spnl_t                 lock;
    struct k_fs_pup_link_t      parent;
    
    union
    {
        struct k_fs_pup_range_t ranges[K_FS_PUP_MAX_RNODE_RANGES];
        struct k_fs_pup_link_t  links[2];
    };
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
    K_FS_PUP_CENTRY_FLAG_LOAD_PENDING = 1 << 1,
};

struct k_fs_pup_centry_t
{
    struct k_fs_pup_centry_t *next;
    struct k_fs_pup_centry_t *prev;
    // k_rt_spnl_t               touch_lock;

    /* how many threads are currently touching this entry. Used by the eviction 
    code to not drop this entry until everyone is done with it. */
    uint32_t                  ref_count;
    k_rt_cond_t *             condition;
    uint32_t                  first_block;
    uint16_t                  flags;
    uint16_t                  align0;
    uint8_t                   buffer[];
};

/* FIXME: cache sets can be local variables, which serve to keep entries alive while code is 
accessing it without needing any sort of synchronization. While this is good for performance
there's the possibility of an entry in a local cache set to become stale compared to the file
system cache (since the local cache set is holding a copy of the blocks), which could cause all 
sorts of funny issues. This needs to be addressed! */
struct k_fs_pup_cset_t
{
    struct k_fs_pup_centry_t *  first_entry;
    struct k_fs_pup_centry_t *  last_entry;
    /* entry currently being inserted in the cache. Used by threads to sniff
    which entry is being inserted in the cache, in case they're looking for it
    but it didn't find in the cache. */
    struct k_fs_pup_centry_t *  cur_insert;

    /* how many threads are currently touching this line. Necessary to keep the
    cache consistent for traversals */
    uint32_t                    read_count;
    /* only one thread at time can modify the cache */
    k_rt_spnl_t                 write_lock;
};

struct k_fs_pup_centry_copy_t
{
    uint32_t entry_buffer_offset;
    uint32_t data_buffer_offset;
    uint32_t copy_size;
};

struct k_fs_pup_vol_t
{
    struct k_fs_pup_root_t    root;
    /* LRU bitmask, used for finding which set has been touched last. The
    last entry in a set's entry linked list is the least recently accessed entry */
    uint32_t                  lru_bitmask;

    /* how much virtual memory is dedicated to this cache. It'll keep growing
    until hitting the max memory allowed for a disk cache. No entry evicion will
    happen as long as the cache is smaller than this. */
    uint32_t                  allocated_memory;

    /* disk cache. It's a N-way set-associative cache. The number of ways 
    per set is variable. The limit of ways is defined by the max memory
    dedicated to a disk cache. Eviction policy is LRU. */
    struct k_fs_pup_cset_t    cache_sets[K_FS_PUP_CACHE_SET_COUNT];

    /* pool of cache entries, kept to avoid touching the kernel memory
    allocator. */
    struct k_fs_pup_centry_t *entry_pool;

    /* in-memory block bitmask, to avoid touching the disk when allocating
    blocks. */
    /* TODO: a more scalable solution is needed. For small volumes this is
    fine, specially if the block size is relatively large. But for larger 
    volumes this will take too much memory. For a 2TB volume, for example,
    this will take around 128MB, which is not great. 
    
    Also, this is pretty slow since it requires a linear scan over an array
    of bytes, and some additional bit twiddling per byte. Something similar
    to the kernel allocator could be used for this when the volume is mounted,
    and the bitmask representation would be used only on disk. */
    uint8_t                  *bitmask_blocks;
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

void k_fs_PupCacheEntry(struct k_fs_pup_vol_t *volume, struct k_fs_pup_cset_t *set, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSetWithCopyFields(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end, struct k_fs_pup_centry_copy_t *copy);

struct k_fs_pup_centry_t *k_fs_PupFindEntryInSet(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint32_t block_start, uint32_t block_end);

void k_fs_PupReleaseEntry(struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntry(struct k_fs_pup_vol_t *volume);

struct k_fs_pup_centry_t *k_fs_PupAllocCacheEntryForAddress(struct k_fs_vol_t *volume, uint32_t address);

void k_fs_PupFreeCacheEntry(struct k_fs_vol_t *volume, struct k_fs_pup_centry_t *entry);

struct k_fs_pup_centry_t *k_fs_PupDropOldestEntry(struct k_fs_pup_vol_t *volume);

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

// void k_fs_PupNextPathComponent(struct k_fs_pup_path_t *path);

struct k_fs_pup_centry_t *k_fs_PupGetBlock(struct k_fs_vol_t *volume, struct k_fs_pup_cset_t *set, uint64_t block_address);

struct k_fs_pup_range_t k_fs_PupAllocBlocks(struct k_fs_vol_t *volume, uint32_t count, uint32_t alloc_type);

void k_fs_PupSetRangeStatus(struct k_fs_vol_t *volume, struct k_fs_pup_range_t *range, uint32_t status);

void k_fs_PupFreeBlock(struct k_fs_pup_range_t block);

struct k_fs_pup_link_t k_fs_PupFindNode(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node, struct k_fs_pup_cset_t *cache);

struct k_fs_pup_node_t *k_fs_PupGetNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t node_address, struct k_fs_pup_cset_t *cache);

struct k_fs_pup_link_t k_fs_PupCreateNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, const char *path);

struct k_fs_pup_link_t k_fs_PupAllocNode(struct k_fs_vol_t *volume, uint32_t type);

void k_fs_PupFreeNode(struct k_fs_pup_link_t link);

struct k_fs_pup_dirlist_t *k_fs_PupGetNodeDirList(struct k_fs_vol_t *volume, const char *path, struct k_fs_pup_link_t start_node);

void k_fs_PupGetPathToNode(struct k_fs_vol_t *volume, struct k_fs_pup_link_t start_node, char *path_buffer, uint32_t buffer_size);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/


// struct k_fs_pup_slot_t *k_fs_PupFindSlot(struct k_fS_vol_t *volume, uint32_t block_address);

void k_fs_PupFlushCache(struct k_fs_vol_t *volume);

#endif
