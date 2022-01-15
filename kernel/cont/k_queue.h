#ifndef K_QUEUE_H
#define K_QUEUE_H

#include "k_defs.h"

struct k_cont_queue_t k_cont_QueueCreate(uint32_t link_offset);

void k_cont_QueueDestroy(struct k_cont_queue_t *queue);

void k_cont_QueuePush(struct k_cont_queue_t *queue, void *element);

void *k_cont_QueuePop(struct k_cont_queue_t *queue);

void k_cont_QueueRemove(struct k_cont_queue_t *queue, void *element);

#endif