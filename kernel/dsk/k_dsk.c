#include "k_dsk.h"
#include "../mem/k_mem.h"
#include "../k_term.h"

struct k_dsk_disk_t *k_dsk_disks = NULL;

void k_dsk_Init(uint32_t boot_drive)
{
    // k_printf("boot drive: %x\n", boot_drive);
}

// struct k_dsk_disk_t *k_dsk_CreateDisk(struct k_dsk_conn_t *connection)
// {
//     (void)connection;
    
//     return NULL;
// }

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    (void)disk;
    (void)start;
    (void)count;
    (void)data;
    return 0;
}

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data)
{
    (void)disk;
    (void)start;
    (void)count;
    (void)data;
    return 0;
}