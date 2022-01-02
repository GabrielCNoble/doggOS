#include "dg_slist.h"
#include "../malloc/dg_malloc.h"
#include "../../kernel/k_term.h"

struct dg_slist_t dg_StackListCreate(uint32_t elem_size, uint32_t buffer_size)
{
    struct dg_slist_t list = {};

    list.elem_size = elem_size;
    list.buffer_size = buffer_size;
    list.free_stack_top = DG_INVALID_INDEX;
    list.size = 0;

    dg_StackListExpand(&list, 1);

    return list;
}

void dg_StackListDestroy(struct dg_slist_t *slist)
{
    if(slist && slist->buffers)
    {
        uint32_t buffer_count = slist->size / slist->buffer_size;

        for(uint32_t buffer_index = 0; buffer_index < buffer_count; buffer_index++)
        {
            dg_Free(slist->buffers[buffer_index]);
        }

        dg_Free(slist->buffers);
        dg_Free(slist->free_stack);

        slist->buffers = NULL;
        slist->buffer_size = 0;
        slist->elem_size = 0;
        slist->free_stack = NULL;
    }
}

uint32_t dg_StackListAllocElement(struct dg_slist_t *slist)
{
    uint32_t elem_index = DG_INVALID_INDEX;

    if(slist && slist->buffers)
    {
        if(slist->free_stack_top != DG_INVALID_INDEX)
        {
            elem_index = slist->free_stack[slist->free_stack_top];
            slist->free_stack_top--;
        }
        else
        {
            if(slist->cursor >= slist->size)
            {
                dg_StackListExpand(slist, slist->buffer_size);        
            }

            elem_index = slist->cursor;
            slist->cursor++;
        }
    }

    return elem_index;
}

void *dg_StackListGetElement(struct dg_slist_t *slist, uint32_t index)
{
    void *element = NULL;

    if(slist && slist->buffers)
    {
        if(index < slist->cursor)
        {
            uint8_t *buffer = slist->buffers[index / slist->buffer_size];
            element = buffer + (index % slist->buffer_size) * slist->elem_size;
        }
    }

    return element;
}

void dg_StackListExpand(struct dg_slist_t *slist, uint32_t elem_count_increase)
{
    if(slist && elem_count_increase)
    {
        uint32_t buffer_count = slist->size / slist->buffer_size;
        uint32_t buffer_count_increase = elem_count_increase / slist->buffer_size;
        uint32_t buffer_index = 0;

        if(!buffer_count_increase)
        {
            buffer_count_increase++;
        }

        void **new_buffers = dg_Malloc(sizeof(void *) * (buffer_count + buffer_count_increase), 4);

        for(; buffer_index < buffer_count; buffer_index++)
        {
            new_buffers[buffer_index] = slist->buffers[buffer_index];
        }

        buffer_count += buffer_count_increase;

        for(; buffer_index < buffer_count; buffer_index++)
        {
            new_buffers[buffer_index] = dg_Malloc(slist->buffer_size, 4);
        }

        if(slist->buffers)
        {
            dg_Free(slist->buffers);
            dg_Free(slist->free_stack);
        }

        slist->buffers = new_buffers;
        slist->size = buffer_count * slist->buffer_size;
        slist->free_stack = dg_Malloc(sizeof(uint32_t) * slist->size, 4);
    }
}