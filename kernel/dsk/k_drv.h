#ifndef K_DRV_H
#define K_DRV_H

#include "defs.h"

struct k_dsk_cmd_page_t *k_dsk_AllocCmdPage();

void k_dsk_QueueCmdPage(struct k_dsk_cmd_page_t *page);

struct k_dsk_cmd_queue_t *k_dsk_GetCmdQueue(uint32_t queue_index);

#endif