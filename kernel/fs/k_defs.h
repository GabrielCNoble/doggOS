#ifndef K_FS_DEFS_H
#define K_FS_DEFS_H

#include <stdint.h>
#include <stddef.h>

#include "../dsk/k_defs.h"


struct k_fs_vol_t;
struct k_fs_fdesc_t
{
    uint32_t handle;
    uint32_t mode;
    struct k_fs_vol_t *volume;
};

struct k_fs_fsys_t
{
    char name[8];
    uint32_t index;
    struct k_fs_fdesc_t *(*create)(struct k_fs_vol_t *volume, char *path);
    void (*destroy)(struct k_fs_vol_t *volume, char *path);
    struct k_fs_fdesc_t *(*open)(struct k_fs_vol_t *volume, char *path);
    void (*close)(struct k_fs_fdesc_t *fdesc);
    uint32_t (*read)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t (*write)(struct k_fs_fdesc_t *fdesc, uint32_t start, uint32_t count, void *data);
    uint32_t (*exists)(struct k_fs_vol_t *volume, char *path);
    uint32_t (*size)(struct k_fs_vol_t *volume, char *path);
};

enum K_FS_PART_TABLE_TYPES
{
    K_FS_PART_TABLE_TYPE_MBR = 0,
    K_FS_PART_TABLE_TYPE_GPT
};

struct k_fs_mbr_part_t
{
    uint8_t boot;
    uint8_t start_head;
    uint16_t start_sec_cyl;
    uint8_t system_id;
    uint8_t end_head;
    uint16_t end_sec_cyl;
    uint16_t start_labl;
    uint16_t start_lbalh;
    uint16_t sec_countl;
    uint16_t sec_counth;
};
struct k_fs_part_t
{
    char name[24];
    struct k_dsk_disk_t *disk;
    uint32_t start;
    uint32_t end;
};
struct k_fs_vol_t
{
    struct k_fs_fsys_t *file_system;
    struct k_fs_part_t *partition;
    uint32_t index;
};

struct k_fs_ptrace_t
{
    char *full_path;
    uint32_t seg_count;
    uint16_t *segs;
};

#endif