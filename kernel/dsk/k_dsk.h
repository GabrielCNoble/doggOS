#ifndef K_DSK_H
#define K_DSK_H

#include "k_dsk_com.h"

struct k_dsk_disk_t *k_dsk_CreateDisk(struct k_dsk_conn_t *connection);

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

#endif