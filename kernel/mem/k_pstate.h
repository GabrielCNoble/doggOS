#ifndef K_PSTATE_H 
#define K_PSTATE_H

#include "k_defs.h" 

struct k_mem_pstate_h k_mem_CreatePState();

void k_mem_MapPState(struct k_mem_pstate_h *pstate, struct k_mem_pstate_t *mapped_pstate);

void k_mem_MapPStateToAddress(struct k_mem_pstate_h *pstate, uint32_t map_address, struct k_mem_pstate_t *mapped_pstate);

void k_mem_UnmapPState(struct k_mem_pstate_t *mapped_pstate);

void k_mem_DestroyPState(struct k_mem_pstate_t *mapped_pstate);

// extern void k_mem_LoadPageDir(uint32_t pdir_page);

void k_mem_LoadPState(struct k_mem_pstate_h *pstate);

extern struct k_mem_pstate_t *k_mem_GetPState();

uint32_t k_mem_MapAddress(struct k_mem_pstate_t *pstate, uint32_t phys_address, uint32_t lin_address, uint32_t flags);

uint32_t k_mem_IsAddressMapped(struct k_mem_pstate_t *pstate, uint32_t address);

uint32_t k_mem_IsPageResident(uint32_t address);

void k_mem_MakePageResident(uint32_t address);

uint32_t k_mem_UnmapAddress(struct k_mem_pstate_t *pstate, uint32_t lin_address);

#endif