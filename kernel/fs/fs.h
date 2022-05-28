#ifndef K_FS_H
#define K_FS_H

#include "defs.h"




void k_fs_Init();

// struct k_fs_fsys_t *k_fs_RegisterFileSystem(struct k_fs_fsys_t *fsys);

// void k_fs_UnregisterFileSystem(char *name);

// struct k_fs_fsys_t *k_fs_GetFileSystem(char *name);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

// void k_fs_EnumerateDiskPartitions(struct k_dsk_disk_t *disk);

struct k_fs_vol_t *k_fs_MountVolume(struct k_fs_part_t *partition);

void k_fs_UnmountVolume(struct k_fs_vol_t *volume);

void k_fs_FormatVolume(struct k_fs_vol_t *volume, void *args);

struct k_fs_vol_t *k_fs_FormatPartition(struct k_fs_part_t *partition, uint32_t file_system, void *args);

void k_fs_FormatDisk(struct k_dsk_disk_t *disk, uint32_t part_table_type);

void k_fs_ReadVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t offset, uint32_t size, void *buffer); 

void k_fs_ReadVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count, void *buffer);

void k_fs_WriteVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t offset, uint32_t size, void *buffer);

void k_fs_WriteVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count, void *buffer);

void k_fs_ClearVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t offset, uint32_t size);

void k_fs_ClearVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_file_t *k_fs_OpenFile(struct k_fs_vol_t *volume, char *path, char *mode);

void k_fs_CloseFile(struct k_fs_file_t *file);

uint32_t k_fs_ReadFile(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);

uint32_t k_fs_WriteFile(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);


#endif