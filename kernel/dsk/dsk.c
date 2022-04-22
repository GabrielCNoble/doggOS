#include "dsk.h"
#include "../mem/mem.h"
#include "../rt/alloc.h"
#include "../proc/thread.h"
#include "../sys/term.h"
// #include "../../libdg/container/dg_slist.h"
// #include "../k_term.h"

// struct k_dsk_disk_t *k_dsk_disks = NULL;

// struct dg_slist_t k_dsk_disks;
struct k_dsk_disk_t *k_dsk_disks = NULL;
struct k_dsk_disk_t *k_dsk_last_disk = NULL;

void k_dsk_Init()
{
    // k_dsk_disks = dg_StackListCreate(sizeof(struct k_dsk_disk_t), 32);
    // (void)boot_drive;
}

struct k_dsk_disk_t *k_dsk_CreateDisk(uint32_t block_size, uint32_t block_count, uint32_t start_address, k_dsk_disk_func_t read, k_dsk_disk_func_t write)
{
    struct k_dsk_disk_t *disk = k_rt_Malloc(sizeof(struct k_dsk_disk_t), 4);
    disk->block_size = block_size;
    disk->block_count = block_count;
    disk->start_address = start_address;
    disk->read = read;
    disk->write = write;
    disk->next = NULL;
    disk->cmds = NULL;
    
    if(!k_dsk_disks)
    {
        k_dsk_disks = disk;
    }
    else
    {
        k_dsk_last_disk->next = disk;
    }
    
    k_dsk_last_disk = disk;
    
    k_sys_TerminalPrintf("create disk %x, block size = %x, block count = %x", disk, block_size, block_count);
    
    disk->thread = k_proc_CreateKernelThread(k_dsk_DiskThread, disk);
    
    return disk;
}

struct k_dsk_cmd_t *k_dsk_AllocCmd()
{
    struct k_dsk_cmd_t *cmd = k_rt_Malloc(sizeof(struct k_dsk_cmd_t), 4);
    cmd->next = NULL;
    cmd->address = 0;
    cmd->size = 0;
    cmd->buffer = NULL;
    cmd->type = K_DSK_CMD_TYPE_LAST;
    cmd->condition = 0;
    // cmd->cmd_condition = 0;
    return cmd;
}

void k_dsk_FreeCmd(struct k_dsk_cmd_t *cmd)
{
    if(cmd)
    {
        k_rt_Free(cmd);
    }
}

void k_dsk_EnqueueCmd(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd)
{
    if(disk && cmd && cmd->type != K_DSK_CMD_TYPE_LAST)
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
// struct k_dsk_disk_t *k_dsk_CreateDisk(struct k_dsk_conn_t *connection)
// {
//     (void)connection;
    
//     return NULL;
// }

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    struct k_dsk_cmd_t *cmd = k_dsk_AllocCmd();
    
    cmd->type = K_DSK_CMD_TYPE_READ;
    cmd->buffer = data;
    cmd->address = start;
    cmd->size = count;
    
    k_dsk_EnqueueCmd(disk, cmd);
    k_proc_WaitCondition(&cmd->condition);
    k_dsk_FreeCmd(cmd);
    
    return 0;
}

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    struct k_dsk_cmd_t *cmd = k_dsk_AllocCmd();
    
    cmd->type = K_DSK_CMD_TYPE_WRITE;
    cmd->buffer = data;
    cmd->address = start;
    cmd->size = count;
    
    k_dsk_EnqueueCmd(disk, cmd);
    k_proc_WaitCondition(&cmd->condition);
    k_dsk_FreeCmd(cmd);
    
    return 0;
}

uint32_t k_dsk_ReadStream(struct k_io_stream_t *stream)
{
    
}

uint32_t k_dsk_WriteStream(struct k_io_stream_t *stream)
{
    
}

uintptr_t k_dsk_DiskThread(void *data)
{
    struct k_dsk_disk_t *disk = (struct k_dsk_disk_t *)data;
    uint32_t index = 0;
    while(1)
    {
        struct k_dsk_cmd_t *cmd = disk->cmds;
        // k_sys_TerminalPrintf("%x\n", index);
        // index++;
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
                case K_DSK_CMD_TYPE_READ:
                    // k_sys_TerminalPrintf("blah\n");
                    disk->read(cmd);
                break;
                
                case K_DSK_CMD_TYPE_WRITE:
                    disk->write(cmd);
                break;
            }
            
            // k_proc_WaitCondition(&cmd->condition);
            // k_dsk_FreeCmd(cmd);
        }
    }
}



