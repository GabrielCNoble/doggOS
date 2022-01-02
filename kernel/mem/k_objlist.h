#ifndef K_OBJ_ALLOC_H
#define K_OBJ_ALLOC_H

#include "k_defs.h"

struct k_mem_objlist_t k_mem_CreateObjList(uint32_t elem_size, uint32_t buffer_size, struct k_mem_sheap_t *heap, uint32_t preserve_free);

void k_mem_DestroyObjList(struct k_mem_objlist_t *list);

uint32_t k_mem_AllocObjListElement(struct k_mem_objlist_t *list);

void k_mem_FreeObjListElement(struct k_mem_objlist_t *list, uint32_t index);

void *k_mem_GetObjListElement(struct k_mem_objlist_t *list, uint32_t index);

void k_mem_AddObjListBuffer(struct k_mem_objlist_t *list, uint32_t buffer_count);

#endif