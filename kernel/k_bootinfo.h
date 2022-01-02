#ifndef K_BOOT_INFO_H
#define K_BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

struct k_boot_info_t
{
    uint32_t start;
    uint32_t size;
};


#endif