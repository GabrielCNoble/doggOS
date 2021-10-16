#ifndef DG_SLIST_H
#define DG_SLIST_H

#include "dg_defs.h"

struct dg_slist_t
{
    uint32_t elem_size;
    uint32_t buffer_size;
    uint32_t size;
    uint32_t cursor;
    void **buffers;
    uint32_t free_stack_top;
    uint32_t *free_stack;
};

struct dg_slist_t dg_StackListCreate(uint32_t elem_size, uint32_t buffer_size);

void dg_StackListDestroy(struct dg_slist_t *slist);

uint32_t dg_StackListAllocElement(struct dg_slist_t *slist);

void *dg_StackListGetElement(struct dg_slist_t *slist, uint32_t index);

void dg_StackListExpand(struct dg_slist_t *slist, uint32_t elem_count_increase);

#endif