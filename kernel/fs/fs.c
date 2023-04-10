#include "fs.h"
#include "../mem/mem.h"
#include "../rt/alloc.h"
#include "../dsk/dsk.h"
#include "../dev/dsk.h"
#include "../sys/term.h"
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
        .format_volume = k_fs_PupFormatVolume,
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

void k_fs_FormatVolume(struct k_fs_vol_t *volume, void *args)
{
    volume->file_system->unmount_volume(volume);
    volume->file_system->format_volume(volume, args);
    volume->file_system->mount_volume(volume);
}

struct k_fs_vol_t *k_fs_FormatPartition(struct k_fs_part_t *partition, uint32_t file_system, void *args)
{
    struct k_fs_vol_t *volume = k_rt_Malloc(sizeof(struct k_fs_vol_t), 4);
    
    volume->next = k_fs_volumes;
    k_fs_volumes = volume;

    volume->partition.disk = partition->disk;
    volume->partition.first_block = partition->first_block;
    volume->partition.block_count = partition->block_count;
    volume->file_system = &k_fs_file_systems[file_system];

    volume->file_system->format_volume(volume, args);
    volume->file_system->mount_volume(volume);

    return volume;
}

void k_fs_ReadVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint64_t first_block, uint64_t offset, uint64_t size, void *buffer)
{
    uint64_t read_start = volume->partition.first_block * volume->partition.disk->block_size;
    uint64_t volume_end = (volume->partition.first_block + volume->partition.block_count) * volume->partition.disk->block_size;
    read_start += first_block * block_size + offset;

    // k_sys_TerminalPrintf("Read %d bytes at %d\n", (uint32_t)size, (uint32_t)read_start);

    // if(read_start + size <= volume_end)
    {
        k_dev_DiskRead(volume->partition.disk, read_start, size, buffer); 
        // k_sys_TerminalPrintf("FUUUUUCK\n");
    }
    // else
    // {
    //     k_sys_TerminalPrintf("first block: %d, block count: %d, block size: %d\n", (uint32_t)volume->partition.first_block, (uint32_t)volume->partition.block_count, (uint32_t)volume->partition.disk->block_size);
    //     k_sys_TerminalPrintf("trying to read past the end of disk! %d\n", (uint32_t)volume_end);
    // }
}

void k_fs_ReadVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint64_t first_block, uint64_t block_count, void *buffer)
{
    // k_sys_TerminalPrintf("k_fs_ReadVolumeBlocks: block_size: %d, first_block: %d, block_count: %d\n", block_size, (uint32_t)first_block, (uint32_t)block_count);
    k_fs_ReadVolumeBytes(volume, block_size, first_block, 0, block_size * block_count, buffer);
}

void k_fs_WriteVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t offset, uint32_t size, void *buffer)
{
    uint32_t write_start = volume->partition.first_block * volume->partition.disk->block_size;
    uint32_t disk_end = (volume->partition.first_block + volume->partition.block_count) * volume->partition.disk->block_size;
    write_start += first_block * block_size + offset;

    // if(write_start + size <= disk_end)
    {
        k_dev_DiskWrite(volume->partition.disk, write_start, size, buffer);
    }
}

void k_fs_WriteVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count, void *buffer)
{
    k_fs_WriteVolumeBytes(volume, block_size, first_block, 0, block_size * block_count, buffer);
}

void k_fs_ClearVolumeBytes(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t offset, uint32_t size)
{
    uint32_t write_start = volume->partition.first_block * volume->partition.disk->block_size;
    uint32_t disk_end = (volume->partition.first_block + volume->partition.block_count) * volume->partition.disk->block_size;
    write_start += first_block * block_size + offset;

    if(write_start + size <= disk_end)
    {
        k_dev_DiskClear(volume->partition.disk, write_start, size);
    }
}

void k_fs_ClearVolumeBlocks(struct k_fs_vol_t *volume, uint32_t block_size, uint32_t first_block, uint32_t block_count)
{
    k_fs_ClearVolumeBytes(volume, block_size, first_block, 0, block_size * block_count);
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