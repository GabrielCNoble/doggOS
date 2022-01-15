#ifndef K_OBJ_ALLOC_H
#define K_OBJ_ALLOC_H

#include "k_defs.h"

struct k_cont_objlist_t k_cont_CreateObjList(uint32_t elem_size, uint32_t buffer_size, struct k_mem_sheap_t *heap, uint32_t preserve_free);

void k_cont_DestroyObjList(struct k_cont_objlist_t *list);

uint32_t k_cont_AllocObjListElement(struct k_cont_objlist_t *list);

void k_cont_FreeObjListElement(struct k_cont_objlist_t *list, uint32_t index);

void *k_cont_GetObjListElement(struct k_cont_objlist_t *list, uint32_t index);

void k_cont_AddObjListBuffer(struct k_cont_objlist_t *list, uint32_t buffer_count);

#endif