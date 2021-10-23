#ifndef K_DSK_H
#define K_DSK_H

#include "k_defs.h"
#include "../init/k_defs.h"

void k_dsk_Init(uint32_t boot_drive);

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

#endif