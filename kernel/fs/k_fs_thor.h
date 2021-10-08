#ifndef K_FS_THOR_H
#define K_FS_THOR_H

#include "k_defs.h"

#define K_FS_THOR_LOGICAL_BLOCK_SIZE 0x1000


/* allows managing a disk of ~562 TB, with block runs of 68 GB per inode,
with 4096 bytes blocks */
struct k_fs_thor_inode_t
{
    uint64_t first_block : 40;
    uint64_t block_count : 24;
};

#endif
