#include "k_bootinfo.h"

extern void *k_kernel_sector;
extern void *k_kernel_size;

union k_boot_info_t k_boot_info = {
    .kernel_start = (uint32_t)&k_kernel_sector, 
    .kernel_size = (uint32_t)&k_kernel_size
};