#include "queue.h"
#include "alloc.h"
#include "atm.h"

/*
=======================================
    based on "Implementing Lock-Free Queues", by John D. Valois
=======================================
*/

struct k_rt_queue_t k_rt_QueueCreate()
{
    struct k_rt_queue_t queue = {};
    queue.prev_head = k_rt_AllocQueueItem(&queue);
    queue.tail = queue.prev_head;
    return queue;
}

void k_rt_AllocQueuePage(struct k_rt_queue_t *queue)
{
    if(queue)
    {
        union k_rt_queue_page_t *new_page = k_rt_BigAlloc(4096, 4096);

        for(uint32_t item_index = 0; item_index < K_RT_QUEUE_PAGE_ITEM_COUNT; item_index++)
        {
            struct k_rt_queue_item_t *item = new_page->items + item_index;
            item->next = new_page->items + item_index + 1;
        }

        struct k_rt_queue_item_t *first_item = new_page->items;
        struct k_rt_queue_item_t *last_item = new_page->items + (K_RT_QUEUE_PAGE_ITEM_COUNT - 1);
        do
        {
            last_item->next = queue->free_items;
        }
        while(!k_rt_CmpXchg((uintptr_t *)&queue->free_items, (uintptr_t)last_item->next, (uintptr_t)first_item, NULL));
    }
}

struct k_rt_queue_item_t *k_rt_AllocQueueItem(struct k_rt_queue_t *queue)
{
    struct k_rt_queue_item_t *item = NULL;
    struct k_rt_queue_item_t *old;

    if(queue)
    {
        item = queue->free_items;

        do
        {
            if(!queue->free_items)
            {
                k_rt_AllocQueuePage(queue);
            }

            item = queue->free_items;
        }
        while(!k_rt_CmpXchg((uintptr_t *)&queue->free_items, (uintptr_t)item, (uintptr_t)item->next, NULL));

        item->next = NULL;
        item->prev = NULL;
        item->item = NULL;

        // k_printf("alloc %x\n", item);
    }

    return item;
}

void k_rt_FreeQueueItem(struct k_rt_queue_t *queue, struct k_rt_queue_item_t *item)
{
    if(queue && item)
    {
        item->next = queue->free_items;
        while(!k_rt_CmpXchg((uintptr_t *)&queue->free_items, (uintptr_t)item->next, (uintptr_t)item, (uintptr_t *)&item->next));
        // k_printf("free %x\n", item);
    }
}

void k_rt_QueueDestroy(struct k_rt_queue_t *queue)
{
    if(queue)
    {

    }
}

void k_rt_QueuePush(struct k_rt_queue_t *queue, void *element)
{
    if(queue && element)
    {
        struct k_rt_queue_item_t *item = k_rt_AllocQueueItem(queue);
        struct k_rt_queue_item_t *tail = queue->tail;
        struct k_rt_queue_item_t *old_tail = tail;

        item->item = element;

        do
        {
            while(tail->next)
            {
                tail = tail->next;
            }
        }
        while(!k_rt_CmpXchg((uintptr_t *)&tail->next, (uintptr_t)NULL, (uintptr_t)item, NULL));
        k_rt_CmpXchg((uintptr_t *)&queue->tail, (uintptr_t)old_tail, (uintptr_t)item, NULL);
    }
}

void k_rt_QueuePushUnsafe(struct k_rt_queue_t *queue, void *element)
{
    (void)queue;
    (void)element;
}

void *k_rt_QueuePop(struct k_rt_queue_t *queue)
{
    void *item = NULL;
    // asm volatile ("cli\n hlt\n");
    __asm__ volatile ("nop\n nop\n");
    if(queue)
    {
        struct k_rt_queue_item_t *prev_head = queue->prev_head;

        do
        {
            if(!prev_head->next)
            {
                return NULL;
            }
        }
        while(!k_rt_CmpXchg((uintptr_t *)&queue->prev_head, (uintptr_t)prev_head, (uintptr_t)prev_head->next, (uintptr_t *)&prev_head));
        item = prev_head->next->item;
        k_rt_FreeQueueItem(queue, prev_head);
    }

    return item;
}

void *k_rt_QueuePopUnsafe(struct k_rt_queue_t *queue)
{
    (void)queue;
    return NULL;
}

void k_rt_QueueRemove(struct k_rt_queue_t *queue, void *element)
{
    (void)queue;
    (void)element;
}
