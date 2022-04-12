#ifndef K_FS_PUP_H
#define K_FS_PUP_H

#include "defs.h"

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
    
    union
    {
        struct k_fs_pup_range_t ranges[K_FS_PUP_MAX_RNODE_RANGES];
        struct k_fs_pup_link_t links[2];
    };
};

void k_fs_InitPupVolume(struct k_fs_vol_t *volume);

#endif
