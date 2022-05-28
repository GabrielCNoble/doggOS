#ifndef K_DSK_H
#define K_DSK_H

#include "defs.h"
#include "../io.h"
// #include "../init/k_defs.h"


/* FIXME: functions to manipulate partition tables (MBR, GUID) */

void k_dsk_Init();

struct k_dsk_disk_t *k_dsk_CreateDisk(struct k_dsk_disk_def_t *disk_def);

struct k_dsk_cmd_t *k_dsk_AllocCmd();

void k_dsk_FreeCmd(struct k_dsk_cmd_t *cmd);

void k_dsk_EnqueueCmd(struct k_dsk_disk_t *disk, struct k_dsk_cmd_t *cmd);

uint32_t k_dsk_Read(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_Write(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dsk_Clear(struct k_dsk_disk_t *disk, uint32_t start, uint32_t count);

uint32_t k_dsk_ReadStream(struct k_io_stream_t *stream);

uint32_t k_dsk_WriteStream(struct k_io_stream_t *stream);

uintptr_t k_dsk_DiskThread(void *data);

#endif