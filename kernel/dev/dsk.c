#include "dsk.h"

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

struct k_dev_dsk_cmd_t *k_dev_AllocDiskCmd()
{
    struct k_dev_dsk_cmd_t *cmd = k_rt_Malloc(sizeof(struct k_dev_dsk_cmd_t), 4);
    cmd->next = NULL;
    cmd->address = 0;
    cmd->size = 0;
    cmd->buffer = NULL;
    cmd->type = K_DEV_DSK_CMD_TYPE_LAST;
    cmd->condition = 0;
    return cmd;
}

void k_dev_FreeDiskCmd(struct k_dev_dsk_cmd_t *cmd)
{
    if(cmd)
    {
        k_rt_Free(cmd);
    }
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

uint32_t k_dev_DiskRead(struct k_dev_disk_t *disk, uint32_t start, uint32_t count, void *data)
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
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd();
    
            cmd->type = K_DEV_DSK_CMD_TYPE_READ;
            cmd->buffer = data;
            cmd->address = start;
            cmd->size = count;
            
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(cmd);
        }
        break;
    }
    
    return 0;
}

uint32_t k_dev_DiskWrite(struct k_dev_disk_t *disk, uint32_t start, uint32_t count, void *data)
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
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd();
    
            cmd->type = K_DEV_DSK_CMD_TYPE_WRITE;
            cmd->buffer = data;
            cmd->address = start;
            cmd->size = count;
            
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(cmd);
        }
        break;
    }
    
    return 0;
}

uint32_t k_dev_DiskClear(struct k_dev_disk_t *disk, uint32_t start, uint32_t count)
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
            struct k_dev_dsk_cmd_t *cmd = k_dev_AllocDiskCmd();
    
            cmd->type = K_DEV_DSK_CMD_TYPE_CLEAR;
            cmd->buffer = NULL;
            cmd->address = start;
            cmd->size = count;
            
            k_dev_EnqueueDiskCmd(disk, cmd);
            k_proc_WaitCondition(&cmd->condition);
            k_dev_FreeDiskCmd(cmd);
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
    k_dev_DeviceReady((struct k_dev_device_t *)data);

    struct k_dev_disk_t *disk = (struct k_dev_disk_t *)data;
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
                    disk->read(disk, cmd);
                break;

                case K_DEV_DSK_CMD_TYPE_CLEAR:
                    disk->clear(disk, cmd);
                break;
                
                case K_DEV_DSK_CMD_TYPE_WRITE:
                    disk->write(disk, cmd);
                break;
            }
        }
    }
}