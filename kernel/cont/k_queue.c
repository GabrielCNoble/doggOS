#include "k_queue.h"

struct k_cont_queue_t k_cont_QueueCreate(uint32_t link_offset)
{
    struct k_cont_queue_t queue = {};
    queue.link_offset = link_offset;
    return queue;
}

void k_cont_QueueDestroy(struct k_cont_queue_t *queue)
{

}

void k_cont_QueuePush(struct k_cont_queue_t *queue, void *element)
{
    if(queue && element)
    {
        struct k_cont_queue_item_t *item = (struct k_cont_queue_item_t *)((char *)element + queue->link_offset);
        struct k_cont_queue_item_t *old_tail;
        do
        {
            item->prev = queue->tail;
        }
        while(!k_atm_CmpXcgh(&queue->tail, item->prev, item, &old_tail));
    }
}

void *k_cont_QueuePop(struct k_cont_queue_t *queue)
{

}

void k_cont_QueueRemove(struct k_cont_queue_t *queue, void *element)
{

}