#include "k_dsk_conn.h"

uint32_t k_dsk_RamRead(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    (void)disk;
    (void)start;
    (void)count;
    (void)data;
    
    return 0;
}

uint32_t k_dsk_RamWrite(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    (void)disk;
    (void)start;
    (void)count;
    (void)data;

    return 0;
}