#ifndef OBJ_LIST_H
#define OBJ_LIST_H

#include <stdint.h>
#include "../mem/defs.h"

struct k_rt_free_obj_t
{
    struct k_rt_free_obj_t *next;
    uint32_t index;
};

struct k_rt_objlist_t
{
    struct k_mem_sheap_t *heap;
    uint32_t elem_size;
    uint32_t buffer_size;
    uint32_t buffer_count;
    uint32_t cursor;
    uint32_t used;
    void **buffers;

    uint32_t preserve_free;
    union
    {
        struct 
        {
            uint32_t free_stack_top;
            uint32_t *free_stack;
        };

        struct 
        {
            struct k_rt_free_obj_t *next_free;
        };
    };
};

struct k_rt_objlist_t k_rt_CreateObjList(uint32_t elem_size, uint32_t buffer_size, struct k_mem_sheap_t *heap, uint32_t preserve_free);

void k_rt_DestroyObjList(struct k_rt_objlist_t *list);

uint32_t k_rt_AllocObjListElement(struct k_rt_objlist_t *list);

void k_rt_FreeObjListElement(struct k_rt_objlist_t *list, uint32_t index);

void *k_rt_GetObjListElement(struct k_rt_objlist_t *list, uint32_t index);

void k_rt_AddObjListBuffer(struct k_rt_objlist_t *list, uint32_t buffer_count);

#endif