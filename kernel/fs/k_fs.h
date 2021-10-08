#ifndef K_FS_H
#define K_FS_H

#include "k_defs.h"




void k_fs_Init();

struct k_fs_rfsys_t *k_fs_RegisterFileSystem(struct k_fs_fsys_t *fsys, char *name);

void k_fs_UnregisterFileSystem(char *name);

struct k_fs_rfsys_t *k_fs_GetFileSystem(char *name);

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_vol_t *k_fs_MountVolume(struct k_dsk_disk_t *disk);

void k_fs_UnmountVolume(struct k_fs_vol_t *volume);

void k_fs_FormatVolume(struct k_fs_vol_t *volume, struct k_fs_rfsys_t *rfsys);

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