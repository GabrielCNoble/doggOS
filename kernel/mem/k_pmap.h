#ifndef K_PMAP_H 
#define K_PMAP_H

#include "k_defs.h" 

struct k_mem_page_map_h k_mem_CreatePageMap();

void k_mem_MapPageMap(struct k_mem_page_map_h *page_map_handle, struct k_mem_page_map_t *page_map);

void k_mem_MapPageMapToAddress(struct k_mem_page_map_h *page_map_handle, uint32_t map_address, struct k_mem_page_map_t *page_map);

void k_mem_UnmapPageMap(struct k_mem_page_map_t *page_map);

void k_mem_DestroyPageMap(struct k_mem_page_map_t *page_map);

void k_mem_LoadPageMap(struct k_mem_page_map_h *page_map_handle);

extern struct k_mem_page_map_t *k_mem_GetPageMap();

uint32_t k_mem_MapAddressOnPageMap(struct k_mem_page_map_t *page_map, uint32_t phys_address, uint32_t lin_address, uint32_t flags);

uint32_t k_mem_MapAddress(uint32_t phys_address, uint32_t lin_address, uint32_t flags);

uint32_t k_mem_IsAddressMapped(struct k_mem_page_map_t *page_map, uint32_t address);

uint32_t k_mem_IsPageResident(uint32_t address);

void k_mem_MakePageResident(uint32_t address);

uint32_t k_mem_UnmapAddress(struct k_mem_page_map_t *page_map, uint32_t lin_address);

#endif