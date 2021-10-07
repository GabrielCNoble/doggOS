#ifndef K_DSK_CONN_H
#define K_DSK_CONN_H

#include "k_dsk_com.h"


uint32_t k_dsk_RamRead(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_RamWrite(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

#endif