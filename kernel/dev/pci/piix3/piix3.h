#ifndef DRV_PIIX3_H
#define DRV_PIIX3_H

#include "../pci.h"

#define K_PIIX3_PCI_TO_ISA_FUNCTION_INDEX 0
#define K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX 1

uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index);

#endif