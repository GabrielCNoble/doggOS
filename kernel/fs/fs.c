#include "fs.h"
#include "../mem/mem.h"
#include "../rt/alloc.h"
#include "../dev/dsk.h"
#include "../rt/mem.h"
#include "../rt/string.h"
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
        .MountVolume        = k_fs_PupMountVolume,
        .FormatVolume       = k_fs_PupFormatVolume,
        .UnmountVolume      = k_fs_PupUnmountVolume,
        .OpenFile           = k_fs_PupOpenFile,
        .CloseFile          = k_fs_PupCloseFile
    }
};

struct k_fs_vol_t *             k_fs_volumes = NULL;

k_rt_spnl_t                     k_fs_open_files_lock = 0;
struct k_fs_file_t *            k_fs_open_files = NULL;
struct k_fs_file_t *            k_fs_last_open_file = NULL;

uint32_t                        k_fs_file_handle_page_index = 0;
uint32_t                        k_fs_file_handle_page_count = 0;
k_rt_spnl_t                     k_fs_file_page_lock = 0;
struct k_fs_file_handle_page_t *k_fs_file_handle_pages = NULL;
struct k_fs_file_handle_page_t *k_fs_cur_file_handle_page = NULL;
struct k_fs_file_handle_page_t *k_fs_last_file_handle_page = NULL;

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
    struct k_fs_vol_t *volume = k_rt_Malloc(sizeof(struct k_fs_vol_t), 0);
    
    volume->next = k_fs_volumes;
    k_fs_volumes = volume;
    
    volume->partition.disk = partition->disk;
    volume->partition.first_block = partition->first_block;
    volume->partition.block_count = partition->block_count;
    volume->file_system = &k_fs_file_systems[K_FS_FILE_SYSTEM_PUP];
    volume->file_system->MountVolume(volume);
    
    return volume;
}

void k_fs_UnmountVolume(struct k_fs_vol_t *volume)
{
    volume->file_system->UnmountVolume(volume);
}

void k_fs_FormatVolume(struct k_fs_vol_t *volume, void *args)
{
    volume->file_system->UnmountVolume(volume);
    volume->file_system->FormatVolume(volume, args);
    volume->file_system->MountVolume(volume);
}

struct k_fs_vol_t *k_fs_FormatPartition(struct k_fs_part_t *partition, uint32_t file_system, void *args)
{
    struct k_fs_vol_t *volume = k_rt_Malloc(sizeof(struct k_fs_vol_t), 0);
    
    volume->next = k_fs_volumes;
    k_fs_volumes = volume;

    volume->partition.disk = partition->disk;
    volume->partition.first_block = partition->first_block;
    volume->partition.block_count = partition->block_count;
    volume->file_system = &k_fs_file_systems[file_system];

    volume->file_system->FormatVolume(volume, args);
    volume->file_system->MountVolume(volume);

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
    (void)mode;


    struct k_fs_file_t *file_handle = k_fs_open_files;

    k_rt_SpinLock(&k_fs_open_files_lock);

    while(file_handle)
    {
        if(!k_rt_StrCmp(path, file_handle->path))
        {
            file_handle->refs++;
            file_handle->volume = volume;
            break;
        }
        file_handle = file_handle->next;
    }

    if(file_handle == NULL)
    {
        file_handle = volume->file_system->OpenFile(volume, path);

        if(file_handle != NULL)
        {
            if(k_fs_open_files == NULL)
            {
                k_fs_open_files = file_handle;
            }
            else
            {
                k_fs_last_open_file->next = file_handle;
                file_handle->prev = k_fs_last_open_file;
            }

            k_fs_last_open_file = file_handle;
        }    

        file_handle->refs = 1;
        file_handle->volume = volume;
    }

    k_rt_SpinUnlock(&k_fs_open_files_lock);

    return file_handle;
}

void k_fs_CloseFile(struct k_fs_file_t *file)
{
    if(file != NULL && file->handle != 0 && file->refs > 0)
    {
        k_rt_SpinLock(&k_fs_open_files_lock);
        file->refs--;
        if(file->refs == 0)
        {
            file->volume->file_system->CloseFile(file);            
            file->handle = 0;
            k_fs_FreeFileHandle(file);
        }
        k_rt_SpinUnlock(&k_fs_open_files_lock);
    }
}

uint32_t k_fs_ReadFile(struct k_fs_file_t *file, uint64_t start, uint64_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}

uint32_t k_fs_WriteFile(struct k_fs_file_t *file, uint64_t start, uint64_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}

struct k_fs_file_t *k_fs_AllocFileHandle()
{
    struct k_fs_file_t *file_handle = NULL;

    if(k_fs_cur_file_handle_page == NULL)
    {
        k_fs_cur_file_handle_page = k_rt_Malloc(K_FS_FILE_HANDLE_PAGE_SIZE, 0);
        k_rt_SetBytes(k_fs_cur_file_handle_page, K_FS_FILE_HANDLE_PAGE_SIZE, 0);
        k_fs_cur_file_handle_page->index = k_fs_file_handle_page_index;
        k_fs_cur_file_handle_page->next_free = k_fs_cur_file_handle_page->entries;
        k_fs_file_handle_page_index++;
        struct k_fs_file_t *next_entry = NULL;
        
        for(uint32_t index = K_FS_FILE_HANDLE_PAGE_ENTRY_COUNT; index > 0;)
        {
            index--;
            struct k_fs_file_t *entry = k_fs_cur_file_handle_page->entries + index;
            entry->next = next_entry;
            entry->page = k_fs_cur_file_handle_page;
            next_entry = entry;
        }

        if(k_fs_file_handle_pages == NULL)
        {
            k_fs_file_handle_pages = k_fs_cur_file_handle_page;
        }
        else
        {
            k_fs_last_file_handle_page->next_page = k_fs_cur_file_handle_page;
            k_fs_cur_file_handle_page->prev_page = k_fs_last_file_handle_page;
        }

        k_fs_last_file_handle_page = k_fs_cur_file_handle_page;
        k_fs_file_handle_page_count++;
    }

    file_handle = k_fs_cur_file_handle_page->next_free;
    k_fs_cur_file_handle_page->next_free = file_handle->next;
    k_fs_cur_file_handle_page->used_count++;
    file_handle->next = NULL;
    file_handle->prev = NULL;

    if(k_fs_cur_file_handle_page->next_free = NULL)
    {
        k_fs_cur_file_handle_page = k_fs_cur_file_handle_page->next_page;
    }

    return file_handle;
}

void k_fs_FreeFileHandle(struct k_fs_file_t *file)
{
    struct k_fs_file_handle_page_t *handle_page = file->page;
    file->next = handle_page->next_free;
    handle_page->next_free = file;
    handle_page->used_count--;

    // if(handle_page->used_count == 0 && k_fs_file_handle_page_count > 1)
    // {

    // }
    // else 
    if(k_fs_cur_file_handle_page == NULL || handle_page->index < k_fs_cur_file_handle_page->index)
    {
        k_fs_cur_file_handle_page = handle_page;
    }
}