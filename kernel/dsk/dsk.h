#ifndef K_DSK_H
#define K_DSK_H

#include "defs.h"
#include "../init/k_defs.h"

void k_dsk_Init();

struct k_dsk_disk_t *k_dsk_CreateDisk(uint32_t block_size, uint32_t block_count, uint32_t start_address, k_dsk_disk_func_t read, k_dsk_disk_func_t write);

struct k_dsk_cmd_t *k_dsk_AllocCmd();

void k_dsk_FreeCmd(struct k_dsk_cmd_t *cmd);

void k_dsk_EnqueueCmd(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd);

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uintptr_t k_dsk_DiskThread(void *data);

#endif