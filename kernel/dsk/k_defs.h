#ifndef K_DSK_DEFS_H
#define K_DSK_DEFS_H

#include <stdint.h> 
#include <stddef.h>

enum K_DSK_CMD_TYPES
{
    K_DSK_CMD_TYPE_READ_START = 0,
    K_DSK_CMD_TYPE_READ_END,
    K_DSK_CMD_TYPE_WRITE_START,
    K_DSK_CMD_TYPE_WRITE_END,
    K_DSK_CMD_TYPE_TRANSFER_PART,
    K_DSK_CMD_TYPE_LAST
};

struct k_dsk_cmd_t
{
    uint32_t type : 6;
    uint32_t cmd_id : 26;
    uint32_t size;
    uint64_t address;
    void *buffer;
};

struct k_dsk_queue_t
{

};

struct k_dsk_part_t
{
    char name[24];
    uint32_t start;
    uint32_t size;
};

enum K_DSK_TYPES
{
    K_DSK_TYPE_RAM = 0,
    K_DSK_TYPE_DISK,
    K_DSK_TYPE_USB,
    K_DSK_TYPE_CDROM,
    K_DSK_TYPE_BOOT
};

struct k_dsk_disk_t
{
    struct k_dsk_disk_t *next;
    struct k_dsk_disk_t *prev;

    uint32_t type;
    uint32_t start_address;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t queue_id;

    struct k_dsk_part_t *partitions;
    struct k_dsk_part_t *primary_partition;
    uint32_t partition_count;
};


#endif