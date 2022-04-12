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

void k_fs_FormatVolume(struct k_fs_vol_t *volume, struct k_fs_fsys_t *fsys);

void k_fs_FormatDisk(struct k_dsk_disk_t *disk, uint32_t part_table_type);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_fdesc_t *k_fs_OpenFile(struct k_fs_vol_t *volume, char *path, char *mode);

void k_fs_CloseFile(struct k_fs_fdesc_t *file);

uint32_t k_fs_ReadFile(struct k_fs_fdesc_t *file, uint32_t start, uint32_t count, void *data);

uint32_t k_fs_WriteFile(struct k_fs_fdesc_t *file, uint32_t start, uint32_t count, void *data);


#endif