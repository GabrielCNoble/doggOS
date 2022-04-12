#include "pup.h"
#include "../rt/alloc.h"
#include "../dsk/dsk.h"
#include "../sys/term.h"

uint32_t k_fs_pup_block_size;
uint8_t *k_fs_pup_disk_buffer;
struct k_fs_pup_root_t *k_fs_pup_root;

void k_fs_InitPupVolume(struct k_fs_vol_t *volume)
{
    uint8_t *disk_buffer = k_rt_Malloc(volume->partition.disk->block_size, 4);
    k_dsk_Read(volume->partition.disk, volume->partition.start * volume->partition.disk->block_size, volume->partition.disk->block_size, disk_buffer);
    k_fs_pup_root = (struct k_fs_pup_root_t *)disk_buffer;
    
    k_fs_pup_block_size = k_fs_pup_root->block_size;
    k_fs_pup_disk_buffer = k_rt_Malloc(k_fs_pup_root->block_size, 4);
    
    // k_sys_TerminalPrintf("volume has block size %d\n", root->block_size);
}

void k_fs_PrintPupVolume(struct k_fs_vol_t *volume)
{
    
}