#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

struct k_rt_queue_item_t
{
    struct k_rt_queue_item_t *next;
    struct k_rt_queue_item_t *prev;
    void *item;
};

#define K_RT_QUEUE_PAGE_ITEM_COUNT (4096 / sizeof(struct k_rt_queue_item_t))

union k_rt_queue_page_t
{
    struct
    {
        struct k_rt_queue_item_t items[K_RT_QUEUE_PAGE_ITEM_COUNT];
    };

    uint8_t bytes[4096];
};

struct k_rt_queue_t
{
    uint32_t                    alloc_count;
    uint32_t                    free_count;
    struct k_rt_queue_item_t *  free_items;
    struct k_rt_queue_item_t *  prev_head;
    struct k_rt_queue_item_t *  tail;
};

struct k_rt_queue_t k_rt_QueueCreate();

void k_rt_AllocQueuePage(struct k_rt_queue_t *queue);

struct k_rt_queue_item_t *k_rt_AllocQueueItem();

void k_rt_FreeQueueItem(struct k_rt_queue_t *queue, struct k_rt_queue_item_t *item);

void k_rt_QueueDestroy(struct k_rt_queue_t *queue);

void k_rt_QueuePush(struct k_rt_queue_t *queue, void *element);

void k_rt_QueuePushUnsafe(struct k_rt_queue_t *queue, void *element);

void *k_rt_QueuePop(struct k_rt_queue_t *queue);

void *k_rt_QueuePopUnsafe(struct k_rt_queue_t *queue);

void k_rt_QueueRemove(struct k_rt_queue_t *queue, void *element);

#endif