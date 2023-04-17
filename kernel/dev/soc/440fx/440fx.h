#ifndef K_440FX_H
#define K_440FX_H

#include "../../dev.h"
#include "../../pci.h"

uint32_t k_440FX_Init(uint8_t bus_index, uint8_t device_index, union k_pci_header_t *header);


#endif