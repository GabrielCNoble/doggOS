#ifndef K_RAM_H
#define K_RAM_H

#include "defs.h"
#include "../dev/dsk.h"

/* FIXME: depending on how big a disk is, it may not be possible to keep it 
completely in memory (if, for example, it was created to back a file system
image). */

uint32_t k_dsk_RamRead(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd);

uint32_t k_dsk_RamWrite(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd);

uint32_t k_dsk_RamClear(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd);

#endif