#ifndef DRV_PIIX3_H
#define DRV_PIIX3_H

#include "pci.h"

uint32_t k_PIIX3_F0_Init(struct k_pci_init_info_t *init_info);

uint32_t k_PIIX3_F1_Init(struct k_pci_init_info_t *init_info);

uint32_t k_PIIX3_F2_Init(struct k_pci_init_info_t *init_info);

#endif