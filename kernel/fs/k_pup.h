#ifndef K_FS_PUP_H
#define K_FS_PUP_H

#include "k_defs.h"

#define K_FS_PUP_LOGICAL_BLOCK_SIZE 0x1000
#define K_FS_PUP_MAX_BLIST_RANGES 

struct k_fs_pup_brange_t
{
    uint64_t first_block : 40;
    uint64_t block_count : 24;
};

struct k_fs_pup_blist_t
{
    uint32_t range_count;
    struct k_fs_pup_brange_t ranges[];
};

/* allows managing a disk of ~562 TB, with block runs of 68 GB per inode,
with 4096 bytes blocks */
// struct k_fs_pup_inode_t
// {
//     uint64_t first_block : 40;
//     uint64_t block_count : 24;
// };

#endif
