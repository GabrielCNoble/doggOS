#ifndef K_RT_POOL_H
#define K_RT_POOL_H

#include <stdint.h>
#include "atm.h"

struct k_rt_pool_page_t;
struct k_rt_pool_t;

union k_rt_pool_node_t
{
    union k_rt_pool_node_t *     next;
    struct k_rt_pool_page_t *    page;
};

#define K_RT_POOL_NODE_NAME pool_node

#define K_RT_POOL_NODE union k_rt_pool_node_t K_RT_POOL_NODE_NAME

#define K_RT_POOL_PAGE_FIELDS                   \
    struct k_rt_pool_page_t *   next;           \
    struct k_rt_pool_page_t *   prev;           \
    union k_rt_pool_node_t *    next_free;      \
    struct k_rt_pool_t *        pool;           \
    uint32_t                    index;          \
    uint32_t                    used_count;     \

#define K_RT_POOL_PAGE_SIZE 4096

struct k_rt_pool_page_t
{
    K_RT_POOL_PAGE_FIELDS;
    uint8_t entries[];
};

struct k_rt_pool_t 
{
    struct k_rt_pool_page_t *    first_page;
    struct k_rt_pool_page_t *    last_page;
    struct k_rt_pool_page_t *    cur_page;
    struct k_rt_pool_page_t *    freeable_pages;
    uint32_t                     next_page_index;
    uint32_t                     obj_size;
    uint32_t                     node_offset;
    uint32_t                     obj_align;
    k_rt_spnl_t                  lock;
};


struct k_rt_pool_t k_rt_pool_Create(uint32_t node_offset, uint32_t obj_size, uint32_t obj_align); 

#define k_rt_pool_CreateForType(obj_type) (k_rt_pool_Create(offsetof(obj_type, K_RT_POOL_NODE_NAME), sizeof(obj_type), alignof(obj_type)))

void k_rt_pool_Destroy(struct k_rt_pool_t *pool);

void *k_rt_pool_AllocObj(struct k_rt_pool_t *pool);

void k_rt_pool_FreeObj(struct k_rt_pool_t *pool, void *obj);

void k_rt_pool_FreeUnusedPages(struct k_rt_pool_t *pool);


#endif