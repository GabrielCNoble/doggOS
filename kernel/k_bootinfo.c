#include "k_bootinfo.h"

extern void *k_kernel_sector;
extern void *k_kernel_size;

struct k_boot_info_t k_boot_info = {
    .start = (uint32_t)&k_kernel_sector, 
    .size = (uint32_t)&k_kernel_size
};