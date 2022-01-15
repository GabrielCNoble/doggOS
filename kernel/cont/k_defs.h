#ifndef K_CONT_DEFS_H
#define K_CONT_DEFS_H

#include "../mem/defs.h"
#include "../atm/k_defs.h"

struct k_cont_free_obj_t
{
    struct k_cont_free_obj_t *next;
    uint32_t index;
};

struct k_cont_objlist_t
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
            struct k_cont_free_obj_t *next_free;
        };
    };
};



struct k_cont_queue_item_t
{
    struct k_cont_queue_item_t *next;
    struct k_cont_queue_item_t *prev;
};

struct k_cont_queue_t
{
    k_atm_spnl_t spinlock;
    uint32_t link_offset;
    struct k_cont_queue_t *head;
    struct k_cont_queue_t *tail;
};

#endif