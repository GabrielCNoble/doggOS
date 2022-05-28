#include "ram.h"
#include "../rt/mem.h"

uint32_t k_dsk_Ram_Read(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd)
{
    uint8_t *disk_buffer = (uint8_t *)disk->start_address;
    k_rt_CopyBytes(cmd->buffer, disk_buffer + cmd->address, cmd->size);
    return 0;
}

uint32_t k_dsk_Ram_Write(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd)
{
    uint8_t *disk_buffer = (uint8_t *)disk->start_address;
    k_rt_CopyBytes(disk_buffer + cmd->address, cmd->buffer, cmd->size);
    return 0;
}

uint32_t k_dsk_Ram_Clear(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd)
{
    uint8_t *disk_buffer = (uint8_t *)disk->start_address;
    // k_rt_CopyBytes(disk_buffer + cmd->address, cmd->buffer, cmd->size);
    // k_rt_Wri
    for(uint32_t index = 0; index < cmd->size; index++)
    {
        disk_buffer[cmd->address + index] = 0;
    }
    
    return 0;
}