#ifndef K_FS_DEFS_H
#define K_FS_DEFS_H

#include <stdint.h>
#include <stddef.h>

#include "../dsk/defs.h"


struct k_fs_vol_t;
struct k_fs_file_t
{
    void              *handle;
    uint32_t           mode;
    struct k_fs_vol_t *volume;
};

struct k_fs_dir_entry_t
{
    char path[64];
};

struct k_fs_dir_t
{
    uint32_t offset;
    struct k_fs_vol_t *volume;
};

enum K_FS_FILE_SYSTEMS
{
    K_FS_FILE_SYSTEM_PUP,
    K_FS_FILE_SYSTEM_FAT32,
    K_FS_FILE_SYSTEM_LAST,
};

struct k_fs_fsys_t
{
    // char                   name[8];
    // uint32_t               index;
    void                     (*mount_volume)(struct k_fs_vol_t *volume);
    void                     (*unmount_volume)(struct k_fs_vol_t *volume);
    struct k_fs_file_t      *(*create_file)(struct k_fs_vol_t *volume, char *path);
    void                     (*destroy_file)(struct k_fs_vol_t *volume, char *path);
    struct k_fs_file_t      *(*open_file)(struct k_fs_vol_t *volume, char *path);
    void                     (*close_file)(struct k_fs_file_t *file);
    uint32_t                 (*read_file)(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);
    uint32_t                 (*write_file)(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);
    uint32_t                 (*file_exists)(struct k_fs_vol_t *volume, char *path);
    uint32_t                 (*file_size)(struct k_fs_vol_t *volume, char *path);
    uint32_t                 (*format_volume)(struct k_fs_vol_t *volume);
    
    struct k_fs_dir_t       *(*open_dir)(struct k_fs_vol_t *volume, char *path);
    void                     (*close_dir)(struct k_fs_dir_t *director);
    struct k_fs_dir_entry_t *(*next_entry)(struct k_fs_dir_t *directory);
    void                     (*rewind_dir)(struct k_fs_dir_t *directory);
    
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
    char                 name[24];
    struct k_dsk_disk_t *disk;
    uint32_t             first_block;
    uint32_t             block_count;
};

struct k_fs_vol_t
{
    struct k_fs_vol_t       *next;
    struct k_fs_fsys_t      *file_system;
    struct k_fs_part_t       partition;
    void                    *data;
};

struct k_fs_ptrace_t
{
    char *full_path;
    uint32_t seg_count;
    uint16_t *segs;
};

#endif