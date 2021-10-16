#include "k_fs.h"
#include "../mem/k_mem.h"

// struct k_fs_rfsys_t *k_fs_rfsys;

void k_fs_Init()
{
    // k_fs_rfsys = NULL;
}

struct k_fs_fsys_t *k_fs_RegisterFileSystem(struct k_fs_fsys_t *fsys)
{
    // struct k_fs_rfsys_t *rfsys = k_fs_GetFileSystem(name);
    
    // if(!rfsys)
    // {
    //     rfsys = k_mem_Malloc(sizeof(struct k_fs_rfsys_t), 0);
    //     rfsys->fsys = *fsys;
    //     uint32_t index;

    //     for(index = 0; index < sizeof(rfsys->name) - 1 || (!name[index]); index++)
    //     {
    //         /* file system names are all lowercase. This is to make sure two different file systems
    //         will have two obviously different names */
    //         rfsys->name[index] = name[index] | 0x20;
    //     }
    //     rfsys->name[index] = '\0';
    //     rfsys->next = k_fs_rfsys;

    //     if(k_fs_rfsys)
    //     {
    //         k_fs_rfsys->prev = rfsys;
    //     }

    //     k_fs_rfsys = rfsys;
    // }

    // return rfsys;
    (void)fsys;
    return NULL;
}

void k_fs_UnregisterFileSystem(char *name)
{
    // struct 
    (void)name;
}

struct k_fs_fsys_t *k_fs_GetFileSystem(char *name)
{
    (void)name;
    return NULL;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_vol_t *k_fs_MountVolume(struct k_dsk_disk_t *disk)
{
    (void)disk;
    return NULL;
}

void k_fs_UnmountVolume(struct k_fs_vol_t *volume)
{
    (void)volume;
}

void k_fs_FormatVolume(struct k_fs_vol_t *volume, struct k_fs_fsys_t *fsys)
{
    (void)volume;
    (void)fsys;
}

/*
=========================================================================================
=========================================================================================
=========================================================================================
*/

struct k_fs_fdesc_t *k_fs_OpenFile(struct k_fs_vol_t *volume, char *path, char *mode)
{
    (void)volume;
    (void)path;
    (void)mode;

    return NULL;
}

void k_fs_CloseFile(struct k_fs_fdesc_t *file)
{
    (void)file;
}

uint32_t k_fs_ReadFile(struct k_fs_fdesc_t *file, uint32_t start, uint32_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}

uint32_t k_fs_WriteFile(struct k_fs_fdesc_t *file, uint32_t start, uint32_t count, void *data)
{
    (void)file;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}