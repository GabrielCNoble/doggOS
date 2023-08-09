#include "pool.h"
#include "mem.h"
#include "alloc.h"

struct k_rt_pool_t k_rt_pool_Create(uint32_t node_offset, uint32_t obj_size, uint32_t obj_align) 
{
    struct k_rt_pool_t pool = {};
    pool.node_offset = node_offset;
    pool.obj_size = obj_size;
    pool.obj_align = obj_align;

    return pool;
}

void k_rt_pool_Destroy(struct k_rt_pool_t *pool)
{
    if(pool != NULL)
    {
        k_rt_pool_FreeUnusedPages(pool);
        while(pool->first_page != NULL)
        {
            struct k_rt_pool_page_t *next_page = pool->first_page->next;
            k_rt_Free(pool->first_page);
            pool->first_page = next_page;
        }
    }
}

void *k_rt_pool_AllocObj(struct k_rt_pool_t *pool) 
{
    void *obj = NULL;

    if(pool != NULL)
    {
        k_rt_SpinLock(&pool->lock);

        struct k_rt_pool_page_t *cur_page = pool->cur_page;

        if(cur_page == NULL)
        {
            if(pool->freeable_pages != NULL)
            {
                cur_page = pool->freeable_pages;
                pool->freeable_pages = cur_page->next;
                cur_page->next = NULL;
                cur_page->prev = NULL;
            }
            else
            {
                cur_page = k_rt_Malloc(K_RT_POOL_PAGE_SIZE, 0);
                uintptr_t entries_block = (uintptr_t)cur_page->entries;
                uint32_t entry_block_size = (K_RT_POOL_PAGE_SIZE - sizeof(struct {K_RT_POOL_PAGE_FIELDS}));
                uintptr_t entries_block_align = entries_block % pool->obj_align;

                if(entries_block_align)
                {
                    entries_block += pool->obj_align - entries_block_align;
                    entry_block_size -= entries_block_align;
                }

                uint32_t entry_count = entry_block_size / pool->obj_size;
                uintptr_t cur_node = entries_block + pool->node_offset;
                cur_page->next_free = (union k_rt_pool_node_t *)cur_node;

                for(uint32_t index = 0; index < entry_count - 1; index++)
                {
                    union k_rt_pool_node_t *entry = (union k_rt_pool_node_t *)cur_node;
                    entry->next = (union k_rt_pool_node_t *)(cur_node + pool->obj_size);
                    cur_node += pool->obj_size;
                }
            }

            pool->cur_page = cur_page;
            
            if(pool->first_page == NULL)
            {
                pool->first_page = cur_page;
            }
            else
            {
                pool->last_page->next = cur_page;
                cur_page->prev = pool->last_page;
            }

            pool->last_page = cur_page;
            cur_page->index = pool->next_page_index;
            pool->next_page_index++;
        }

        union k_rt_pool_node_t *node = cur_page->next_free;
        obj = (void *)((uintptr_t)node - pool->node_offset);
        cur_page->next_free = node->next;
        cur_page->used_count++;

        k_rt_SpinUnlock(&pool->lock);

        k_rt_SetBytes(obj, pool->obj_size, 0);
        node->page = cur_page;
    }

    return obj;
}

void k_rt_pool_FreeObj(struct k_rt_pool_t *pool, void *obj)
{
    if(pool != NULL && obj != NULL)
    {
        union k_rt_pool_node_t *node = (union k_rt_pool_node_t *)((uintptr_t)obj + pool->node_offset);
        k_rt_SpinLock(&pool->lock);

        struct k_rt_pool_page_t *page = node->page;
        node->next = page->next_free;
        page->next_free = node;
        page->used_count--;

        if(pool->cur_page == NULL || page->index < pool->cur_page->index)
        {
            pool->cur_page = page;
        }
        else if(page->used_count == 0)
        {
            if(page == pool->first_page)
            {
                pool->first_page = page->next;
            }
            else
            {
                page->prev->next = page->next;
            }

            if(page == pool->last_page)
            {
                pool->last_page = page->prev;
            }
            else
            {
                page->next->prev = page->prev;
            }

            page->prev = NULL;
            page->next = pool->freeable_pages;
            pool->freeable_pages = page;
        }

        k_rt_SpinUnlock(&pool->lock);
    }
}

void k_rt_pool_FreeUnusedPages(struct k_rt_pool_t *pool)
{
    if(pool != NULL)
    {
        k_rt_SpinLock(&pool->lock);
        while(pool->freeable_pages != NULL)
        {
            struct k_rt_obj_page_t *next_page = pool->freeable_pages->next;
            k_rt_Free(pool->freeable_pages);
            pool->freeable_pages = next_page;
        }
        k_rt_SpinUnlock(&pool->lock);
    }
}