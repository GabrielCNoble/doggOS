#ifndef K_FS_COM_H
#define K_FS_COM_H

#include <stdint.h>
#include <stddef.h>

#include "../fs/k_fs_com.h"
#include "../dsk/k_dsk_com.h"

struct k_fs_fdesc_t
{
    uint32_t handle;
    uint32_t mode;
    struct k_fs_vol_t *volume;
};

/* file system definition */
struct k_fs_fsys_t
{
    struct k_fs_fdesc_t *(*create)(char *path);
    void                 (*destroy)(char *path);
    struct k_fs_fdesc_t *(*open)(struct k_fs_vol_t *volume, char *path);
    void                 (*close)(struct k_fs_fdesc_t *fdesc);
    uint32_t             (*read)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t             (*write)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t             (*exists)(char *path);
    uint32_t             (*size)(char *path);
};

/* registered file system */
struct k_fs_rfsys_t
{
    struct k_fs_rfsys_t *next;
    struct k_fs_rfsys_t *prev;
    char name[8];
    struct k_fs_fsys_t fsys;
};

struct k_fs_vol_t
{
    struct k_fs_vol_t *next;
    struct k_fs_vol_t *prev;
    struct k_fs_rfsys_t *rfsys;
    struct k_dsk_disk_t *disk;
};

#endif