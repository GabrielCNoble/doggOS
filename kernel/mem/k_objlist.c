#include "k_objlist.h"
#include "k_alloc.h"
#include "../../libdg/string/dg_string.h"

struct k_mem_objlist_t k_mem_CreateObjList(uint32_t elem_size, uint32_t buffer_size, struct k_mem_sheap_t *heap, uint32_t preserve_free)
{
    struct k_mem_objlist_t obj_list = {};

    if(elem_size && buffer_size && heap)
    {
        if(!preserve_free)
        {
            if(elem_size < sizeof(struct k_mem_free_obj_t))
            {
                elem_size = sizeof(struct k_mem_free_obj_t);
            }
        }
        else
        {
            obj_list.free_stack_top = 0xffffffff;
        }

        obj_list.heap = heap;
        obj_list.elem_size = elem_size;
        obj_list.buffer_size = buffer_size;
        obj_list.preserve_free = preserve_free;

        k_mem_AddObjListBuffer(&obj_list, 1);
    }

    return obj_list;
}

void k_mem_DestroyObjList(struct k_mem_objlist_t *list)
{
    if(list && list->buffers)
    {
        for(uint32_t buffer_index = 0; buffer_index < list->buffer_count; buffer_index++)
        {
            k_mem_Free(list->heap, list->buffers[buffer_index]);
        }

        k_mem_Free(list->heap, list->buffers);

        if(list->preserve_free)
        {
            k_mem_Free(list->heap, list->free_stack);
        }

        list->preserve_free = 0;
        list->buffers = NULL;
        list->buffer_size = 0;
        list->buffer_count = 0;
        list->cursor = 0;
        list->used = 0;
        list->elem_size = 0;
        list->heap = NULL;
    }
}

uint32_t k_mem_AllocObjListElement(struct k_mem_objlist_t *list)
{
    uint32_t index = 0xffffffff;

    if(list && list->buffers)
    {
        if(list->preserve_free && list->free_stack_top != 0xffffffff)
        {
            index = list->free_stack[list->free_stack_top];
            list->free_stack_top--;
        }
        else if(!list->preserve_free && list->next_free)
        {   
            struct k_mem_free_obj_t *next_free = list->next_free;
            list->next_free = next_free->next;
            index = next_free->index;
        }
        else
        {
            index = list->cursor;
            list->cursor++;

            if(list->cursor == list->buffer_size * list->buffer_count)
            {
                k_mem_AddObjListBuffer(list, 1);
            }
        }

        list->used++;
    }

    return index;
}

void k_mem_FreeObjListElement(struct k_mem_objlist_t *list, uint32_t index)
{
    void *element = k_mem_GetObjListElement(list, index);

    if(element)
    {
        if(list->preserve_free)
        {
            list->free_stack_top++;
            list->free_stack[list->free_stack_top] = index;
        }
        else
        {
            struct k_mem_free_obj_t *free_obj = (struct k_mem_free_obj_t *)element;
            free_obj->index = index;
            free_obj->next = list->next_free;
            list->next_free = free_obj;
        }

        list->used--;
    }
}

void *k_mem_GetObjListElement(struct k_mem_objlist_t *list, uint32_t index)
{
    void *element = NULL;
    k_printf("element %d\n", index);
    if(list && list->buffers && index < list->cursor)
    {
        void *buffer = list->buffers[index / list->buffer_size];
        element = (void *)((char *)buffer + (index % list->buffer_size) * list->elem_size);
    }

    return element;
}

void k_mem_AddObjListBuffer(struct k_mem_objlist_t *list, uint32_t buffer_count)
{
    if(list && buffer_count)
    {
        uint32_t new_buffer_count = list->buffer_count + buffer_count;
        void **buffer_list = k_mem_Malloc(list->heap, sizeof(void *) * new_buffer_count, sizeof(void *));

        if(list->buffers)
        {
            ds_CopyBytes(buffer_list, list->buffers, sizeof(void *) * list->buffer_count);
            k_mem_Free(list->heap, list->buffers);
        }

        list->buffers = buffer_list;

        if(list->preserve_free)
        {
            if(list->free_stack)
            {
                k_mem_Free(list->heap, list->free_stack);
            }

            list->free_stack = k_mem_Malloc(list->heap, sizeof(uint32_t) * new_buffer_count * list->buffer_size, 4);
        }

        for(uint32_t buffer_index = list->buffer_count; buffer_index < new_buffer_count; buffer_index++)
        {
            list->buffers[buffer_index] = k_mem_Malloc(list->heap, list->buffer_size, list->elem_size);
        }

        list->buffer_count = new_buffer_count;
    }
}