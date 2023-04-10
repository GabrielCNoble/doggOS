#include "dsk.h"
#include "../rt/mem.h"
#include "../sys/sys.h"
#include "../sys/term.h"

struct k_dev_disk_t *k_dev_disks = NULL;
struct k_dev_disk_t *k_dev_last_disk = NULL;

// struct k_dev_disk_t *k_dev_CreateDisk(struct k_dev_disk_def_t *disk_def)
// {
//     // struct k_dev_device_desc_t device_desc = {
//         // .device_type = K_DEV_DEVICE_TYPE_DISK,
//         // .device_size = sizeof(struct k_dev_disk_t),
//         // .driver_func = k_dev_DiskThread
//     // };

//     struct k_dev_device_t *disk_device = k_dev_CreateDevice((struct k_dev_desc_t *)device_desc);

//     if(disk_device != NULL)
//     {
//         struct k_dev_disk_t *disk = (struct k_dev_disk_t *)disk_device;
//         disk->def = *disk_def;
//     }
// }

struct k_dev_dsk_cmd_t *k_dev_AllocDiskCmd(struct k_dev_disk_t *disk)
{
    k_rt_SpinLockCritical(&disk->cmd_page_lock);

    struct k_dev_dsk_cmd_page_t *cmd_page = disk->cur_cmd_page;

    if(cmd_page->next_free == NULL)
    {
        while(cmd_page != NULL && cmd_page->next_free == NULL)
        {
            cmd_page = cmd_page->next;
        }

        if(cmd_page == NULL)
        {
            if(disk->freeable_cmd_pages != NULL)
            {
                cmd_page = disk->freeable_cmd_pages;
                disk->freeable_cmd_pages = cmd_page->next;
            }
            else
            {
                cmd_page = k_rt_Malloc(sizeof(struct k_dev_dsk_cmd_page_t), 0);

                if(cmd_page == NULL)
                {
                    k_sys_RaiseException(K_EXCEPTION_FAILED_MEMORY_ALLOCATION);
                }
            }

            k_dev_InitDiskCmdPage(disk, cmd_page);
        }

        disk->cur_cmd_page = cmd_page;
    }

    struct k_dev_dsk_cmd_t *cmd = disk->cur_cmd_page->next_free;
    disk->cur_cmd_page->next_free = cmd->next;
    disk->cur_cmd_page->used_count++;

    k_rt_SpinUnlockCritical(&disk->cmd_page_lock);
    cmd->buffer = NULL;
    cmd->condition = 0;
    cmd->next = NULL;
    cmd->address = 0;
    cmd->type = K_DEV_DSK_CMD_TYPE_LAST;
    cmd->size = 0;
    cmd->status = 0;

    // k_rt_SetBytes(cmd, disk->cmd_size, 0);

    return cmd;
}

void k_dev_FreeDiskCmd(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd)
{
    if(disk != NULL && cmd != NULL)
    {
        k_rt_SpinLockCritical(&disk->cmd_page_lock);
        struct k_dev_dsk_cmd_page_t *cmd_page = cmd->page;

        cmd->next = cmd_page->next_free;
        cmd_page->next_free = cmd;
        cmd_page->used_count--;

        if(cmd_page->used_count == 0 && disk->cmd_page_count > 1)
        {
            if(cmd_page == disk->cmd_pages)
            {
                disk->cmd_pages = cmd_page->next;
                disk->cmd_pages->prev = NULL;
            }
            else
            {
                cmd_page->prev->next = cmd_page->next;
            }

            if(cmd_page == disk->last_cmd_page)
            {
                disk->last_cmd_page = cmd_page->prev;
                disk->last_cmd_page->next = NULL;
            }
            else
            {
                cmd_page->next->prev = cmd_page->prev;
            }

            cmd_page->next = disk->freeable_cmd_pages;
            disk->freeable_cmd_pages = cmd_page;
            disk->cmd_page_count--;
        }
        else if(cmd_page->index < disk->cur_cmd_page->index)
        {
            disk->cur_cmd_page = cmd_page;
        }

        k_rt_SpinUnlockCritical(&disk->cmd_page_lock);
    }
}

void k_dev_InitDiskCmdPage(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_page_t *cmd_page)
{
    uintptr_t cmd_start = (uintptr_t)cmd_page->cmd_data;
    uintptr_t aligned_cmd_start;
    uintptr_t cmd_align = (uintptr_t)(disk->cmd_align - 1);
    aligned_cmd_start = (cmd_start + cmd_align) & ~cmd_align;
    uint32_t cmd_count = (sizeof(cmd_page->cmd_data) - (uint32_t)(aligned_cmd_start - cmd_start)) / (uint32_t)disk->cmd_size;

    struct k_dev_dsk_cmd_t *cmd = NULL;
    cmd_page->used_count = 0;
    cmd_page->index = disk->cmd_page_index;
    cmd_page->next_free = (struct k_dev_dsk_cmd_t *)aligned_cmd_start;
    disk->cmd_page_index++;

    for(uint32_t index = 0; index < cmd_count; index++)
    {
        cmd = (struct k_dev_dsk_cmd_t *)aligned_cmd_start;
        aligned_cmd_start += disk->cmd_size;
        struct k_dev_dsk_cmd_t *next_cmd = (struct k_dev_dsk_cmd_t *)aligned_cmd_start;
        cmd->next = next_cmd;
        cmd->page = cmd_page;
        cmd->type = K_DEV_DSK_CMD_TYPE_LAST;
        cmd->buffer = NULL;
    }

    cmd->next = NULL;

    if(disk->cmd_pages == NULL)
    {
        disk->cmd_pages = cmd_page;
    }
    else
    {
        disk->last_cmd_page->next = cmd_page;
        cmd_page->prev = disk->last_cmd_page;
    }

    disk->last_cmd_page = cmd_page;
}

void k_dev_EnqueueDiskCmd(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd)
{
    if(disk && cmd && cmd->type != K_DEV_DSK_CMD_TYPE_LAST)
    {
        if(!disk->cmds)
        {
            disk->cmds = cmd;
        }
        else
        {
            disk->last_cmd->next = cmd;
        }
        
        disk->last_cmd = cmd;
    }
}

uint32_t k_dev_DiskRead(struct k_dev_disk_t *disk, uint64_t start, uint64_t count, void *data)
{
    switch(disk->type)
    {
        case K_DEV_DSK_TYPE_RAM:
        {
            struct k_dev_dsk_cmd_t cmd = {};            
            cmd.buffer = data;
            cmd.address = start;
            cmd.size = count;
            disk->read(disk, &cmd);
        }
        break;

        case K_DEV_DSK_TYPE_DISK:
        {
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd(disk);   
            cmd->type = K_DEV_DSK_CMD_TYPE_READ;
            cmd->buffer = data;
            cmd->address = start;
            cmd->size = count;
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(disk, cmd);
        }
        break;
    }
    
    return 0;
}

uint32_t k_dev_DiskWrite(struct k_dev_disk_t *disk, uint64_t start, uint64_t count, void *data)
{
    switch(disk->type)
    {
        case K_DEV_DSK_TYPE_RAM:
        {
            struct k_dev_dsk_cmd_t cmd = {};            
            cmd.buffer = data;
            cmd.address = start;
            cmd.size = count;
            disk->write(disk, &cmd);
        }
        break;

        case K_DEV_DSK_TYPE_DISK:
        {
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd(disk);
    
            cmd->type = K_DEV_DSK_CMD_TYPE_WRITE;
            cmd->buffer = data;
            cmd->address = start;
            cmd->size = count;
            
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(disk, cmd);
        }
        break;
    }
    
    return 0;
}

uint32_t k_dev_DiskClear(struct k_dev_disk_t *disk, uint64_t start, uint64_t count)
{
    switch(disk->type)
    {
        case K_DEV_DSK_TYPE_RAM:
        {
            struct k_dev_dsk_cmd_t cmd = {};            
            cmd.buffer = NULL;
            cmd.address = start;
            cmd.size = count;
            disk->clear(disk, &cmd);
        }
        break;

        case K_DEV_DSK_TYPE_DISK:
        {
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd(disk);
    
            cmd->type = K_DEV_DSK_CMD_TYPE_CLEAR;
            cmd->buffer = NULL;
            cmd->address = start;
            cmd->size = count;
            
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(disk, cmd);
        }
        break;
    }
    
    return 0;
}

uint32_t k_dev_DiskReadStream(struct k_io_stream_t *stream)
{
    
}

uint32_t k_dev_DiskWriteStream(struct k_io_stream_t *stream)
{
    
}

uintptr_t k_dev_DiskThread(void *data)
{
    struct k_dev_disk_t *disk = (struct k_dev_disk_t *)data;
    k_dev_DeviceReady((struct k_dev_device_t *)data);

    // k_cpu_DisableInterrupts();
    // k_cpu_Halt();        

    uint32_t index = 0;
    while(1)
    {
        struct k_dev_dsk_cmd_t *cmd = disk->cmds;

        if(!cmd)
        {
            
            k_proc_YieldThread();
            
        }
        else
        {
            disk->cmds = disk->cmds->next;
            
            if(!disk->cmds)
            {
                disk->last_cmd = NULL;
            }
            
            switch(cmd->type)
            {
                case K_DEV_DSK_CMD_TYPE_READ:
                    // k_sys_TerminalPrintf("read some shit...\n");
                    
                    disk->read(disk, cmd);
                    
                    // k_sys_TerminalPrintf("done reading some shit...\n");
                break;

                case K_DEV_DSK_CMD_TYPE_CLEAR:
                    disk->clear(disk, cmd);
                break;
                
                case K_DEV_DSK_CMD_TYPE_WRITE:
                    disk->write(disk, cmd);
                break;
            }

            // k_proc_WaitCondition(&cmd->condition);
        }
    }
} 