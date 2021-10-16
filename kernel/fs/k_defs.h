#ifndef K_FS_DEFS_H
#define K_FS_DEFS_H

#include <stdint.h>
#include <stddef.h>

#include "../dsk/k_defs.h"


struct k_fs_vol_t;
struct k_fs_fdesc_t;
struct k_fs_fsys_t;
struct k_fs_fdesc_t
{
    uint32_t handle;
    uint32_t mode;
    struct k_fs_vol_t *volume;
};

struct k_fs_fsys_t
{
    char name[8];
    struct k_fs_fsys_t *next;
    struct k_fs_fsys_t *prev;

    struct k_fs_fdesc_t *(*create)(char *path);
    void (*destroy)(char *path);
    struct k_fs_fdesc_t *(*open)(struct k_fs_vol_t *volume, char *path);
    void (*close)(struct k_fs_fdesc_t *fdesc);
    uint32_t (*read)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t (*write)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t (*exists)(char *path);
    uint32_t (*size)(char *path);
};

struct k_fs_vol_t
{
    struct k_fs_vol_t *next;
    struct k_fs_vol_t *prev;
    struct k_fs_fsys_t *fsys;
    struct k_dsk_disk_t *disk;
};

struct k_fs_ptrace_t
{
    char *full_path;
    uint32_t seg_count;
    uint16_t *segs;
};

#endif