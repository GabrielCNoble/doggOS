#ifndef K_FS_DEFS_H
#define K_FS_DEFS_H

#include <stdint.h>
#include <stddef.h>

#include "../dsk/defs.h"
#include "../dev/dsk.h"


/* forward declarations */
struct k_fs_vol_t;
struct k_fs_file_handle_page_t;


struct k_fs_file_t
{
    uint32_t                                mode;
    uint32_t                                refs;
    struct k_fs_vol_t *                     volume;
    struct k_fs_file_handle_page_t *        page;
    uint64_t                                handle;
    struct k_fs_file_t *                    next;
    struct k_fs_file_t *                    prev;
    char *                                  path;
};

#define K_FS_FILE_HANDLE_PAGE_FIELDS                    \
    uint32_t                            index;          \
    struct k_fs_file_handle_page_t *    next_page;      \
    struct k_fs_file_handle_page_t *    prev_page;      \
    struct k_fs_file_t *                next_free;      \
    uint32_t                            used_count;

#define K_FS_FILE_HANDLE_PAGE_SIZE 4096
#define K_FS_FILE_HANDLE_PAGE_ENTRY_COUNT ((K_FS_FILE_HANDLE_PAGE_SIZE - sizeof(struct {K_FS_FILE_HANDLE_PAGE_FIELDS})) / sizeof(struct k_fs_file_t))

struct k_fs_file_handle_page_t
{
    K_FS_FILE_HANDLE_PAGE_FIELDS;
    struct k_fs_file_t entries[K_FS_FILE_HANDLE_PAGE_ENTRY_COUNT];
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
    void                        (*MountVolume)(struct k_fs_vol_t *volume);
    uint32_t                    (*FormatVolume)(struct k_fs_vol_t *volume, void *args);
    void                        (*UnmountVolume)(struct k_fs_vol_t *volume);
    struct k_fs_file_t *        (*CreateFile)(struct k_fs_vol_t *volume, char *path);
    void                        (*DestroyFile)(struct k_fs_vol_t *volume, char *path);
    struct k_fs_file_t *        (*OpenFile)(struct k_fs_vol_t *volume, char *path);
    void                        (*CloseFile)(struct k_fs_file_t *file);
    uint32_t                    (*ReadFile)(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);
    uint32_t                    (*WriteFile)(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data);
    uint32_t                    (*FileExists)(struct k_fs_vol_t *volume, char *path);
    uint32_t                    (*FileSize)(struct k_fs_file_t *file);
    
    struct k_fs_dir_t *         (*OpenDir)(struct k_fs_vol_t *volume, char *path);
    void                        (*CloseDir)(struct k_fs_dir_t *director);
    struct k_fs_dir_entry_t *   (*NextEntry)(struct k_fs_dir_t *directory);
    void                        (*RewindDir)(struct k_fs_dir_t *directory);
    
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

/* FIXME: this should probably be part of the disk interface. */
struct k_fs_part_t
{
    char                        name[24];
    struct k_dev_disk_t *       disk;
    uint32_t                    first_block;
    uint32_t                    block_count;
};

struct k_fs_vol_t
{
    struct k_fs_vol_t       *next;
    struct k_fs_fsys_t      *file_system;
    struct k_fs_part_t       partition;
    void                    *data;
};

// struct k_fs_path_state_t
// {
//     char *full_path;
//     uint32_t segment_start;
//     uint32_t segment_end;
// };

#endif