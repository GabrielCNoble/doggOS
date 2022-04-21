#include "fs.h"
#include "../mem/mem.h"
#include "../rt/alloc.h"
#include "pup.h"
// #include "../../libdg/container/dg_slist.h"
// #include "../../libdg/string/dg_string.h"

// struct k_fs_rfsys_t *k_fs_rfsys;

// struct dg_slist_t k_fs_file_systems;
// struct dg_slist_t k_fs_volumes;
// struct dg_slist_t k_fs_partitions;

struct k_fs_fsys_t k_fs_file_systems[K_FS_FILE_SYSTEM_LAST] = {
    [K_FS_FILE_SYSTEM_PUP] = {
        .mount_volume = k_fs_PupMountVolume,
        .unmount_volume = k_fs_PupUnmountVolume
    }
};

struct k_fs_vol_t *k_fs_volumes = NULL;

void k_fs_Init()
{
    // k_fs_file_systems = dg_StackListCreate(sizeof(struct k_fs_fsys_t), 8);
    // k_fs_volumes = dg_StackListCreate(sizeof(struct k_fs_vol_t), 16);
    // k_fs_partitions = dg_StackListCreate(sizeof(struct k_fs_part_t), 16);
}

// struct k_fs_fsys_t *k_fs_RegisterFileSystem(struct k_fs_fsys_t *file_system)
// {
//     (void)file_system;
//     struct k_fs_fsys_t *new_file_system = NULL;
// 
//     // if(file_system)
//     // {
//     //     new_file_system = k_fs_GetFileSystem(file_system->name);
// 
//     //     if(!new_file_system)
//     //     {
//     //         uint32_t index = dg_StackListAllocElement(&k_fs_file_systems);
//     //         new_file_system = dg_StackListGetElement(&k_fs_file_systems, index);
//     //         ds_CopyBytes(new_file_system, file_system, sizeof(struct k_fs_fsys_t));
//     //         new_file_system->index = index;
//     //     }
//     // }
// 
//     return new_file_system;
// }
// 
// void k_fs_UnregisterFileSystem(char *name)
// {
//     // struct 
//     (void)name;
// }
// 
// struct k_fs_fsys_t *k_fs_GetFileSystem(char *name)
// {
//     (void)name;
//     struct k_fs_fsys_t *file_system = NULL;
// 
//     // for(uint32_t index = 0; index < k_fs_file_systems.cursor; index++)
//     // {
//     //     file_system = dg_StackListGetElement(&k_fs_file_systems, index);
// 
//     //     if(file_system && file_system->index == DG_INVALID_INDEX)
//     //     {
//     //         file_system = NULL;
//     //         continue;
//     //     }
// 
//     //     break;
//     // }
// 
//     return file_system;
// }

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_vol_t *k_fs_MountVolume(struct k_fs_part_t *partition)
{
    // (void)partition;
    struct k_fs_vol_t *volume = k_rt_Malloc(sizeof(struct k_fs_vol_t), 4);
    
    volume->next = k_fs_volumes;
    k_fs_volumes = volume;
    
    volume->partition.disk = partition->disk;
    volume->partition.first_block = partition->first_block;
    volume->partition.block_count = partition->block_count;
    volume->file_system = &k_fs_file_systems[K_FS_FILE_SYSTEM_PUP];
    
    volume->file_system->mount_volume(volume);
    
    return volume;
}

void k_fs_UnmountVolume(struct k_fs_vol_t *volume)
{
    volume->file_system->unmount_volume(volume);
}

void k_fs_FormatVolume(struct k_fs_vol_t *volume, struct k_fs_fsys_t *fsys)
{
    (void)volume;
    (void)fsys;
}

void k_fs_ReadVolume(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count, void *buffer)
{
    uint32_t read_count = block_count * block_size;
    uint32_t read_start = volume->partition.first_block * volume->partition.disk->block_size;
    read_start += first_block * block_size;
    k_dsk_Read(volume->partition.disk, read_start, read_count, buffer); 
}

void k_fs_WriteVolume(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count, void *buffer)
{
    
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_file_t *k_fs_OpenFile(struct k_fs_vol_t *volume, char *path, char *mode)
{
    (void)volume;
    (void)path;
    (void)mode;

    return NULL;
}

void k_fs_CloseFile(struct k_fs_file_t *file)
{
    (void)file;
}

uint32_t k_fs_ReadFile(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}

uint32_t k_fs_WriteFile(struct k_fs_file_t *file, uint32_t start, uint32_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}