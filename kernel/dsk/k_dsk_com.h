#ifndef K_DSK_COM_H
#define K_DSK_COM_H

#include <stdint.h>
#include <stddef.h>

enum K_DSK_CONN_TYPES
{
    K_DSK_CONN_TYPE_MEMORY = 0,
    K_DSK_CONN_TYPE_DISK,
    K_DSK_CONN_TYPE_USB
};

/* forward declaration */
struct k_dsk_disk_t;

struct k_dsk_conn_t
{
    uint32_t type;
    uint32_t (*read)(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);
    uint32_t (*write)(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

    union
    {
        struct { uint32_t start_address; uint32_t block_size; } ram_conn;
        struct { uint32_t start_sector; uint32_t sector_count; } disk_conn;
    };
};

struct k_dsk_part_t
{
    char name[24];
    uint32_t start;
    uint32_t size;
};

struct k_dsk_disk_t
{
    struct k_dsk_disk_t *next;
    struct k_dsk_disk_t *prev;

    struct k_dsk_conn_t connection;

    struct k_dsk_part_t *partitions;
    struct k_dsk_part_t *primary_partition;
    uint32_t partition_count;
};


#endif