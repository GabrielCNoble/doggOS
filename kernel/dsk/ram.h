#ifndef K_RAM_H
#define K_RAM_H

#include "defs.h"

/* FIXME: depending on how big a disk is, it may not be possible to keep it 
completely in memory (if, for example, it was created to back a file system
image). */

uint32_t k_dsk_Ram_Read(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd);

uint32_t k_dsk_Ram_Write(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd);

uint32_t k_dsk_Ram_Clear(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd);

#endif