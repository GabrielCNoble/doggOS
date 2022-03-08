#ifndef RBUFFER_H
#define RBUFFER_H

#include <stdint.h>


struct k_rt_rbuffer_t
{
    void **buffer;
    uint32_t next_out;
    uint32_t next_in;
};

struct k_rt_rbuffer_t k_rt_RingBufferCreate(uint32_t buffer_size);

void k_rt_RingBufferDestroy(struct k_rt_rbuffer_t *ring_buffer);

uint32_t k_rt_RingBufferPush(struct k_rt_rbuffer_t *ring_buffer, void *data);

void *k_rt_RingBufferPop(struct k_rt_rbuffer_t *ring_buffer);

#endif