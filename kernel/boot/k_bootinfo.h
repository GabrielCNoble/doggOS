#ifndef K_BOOT_INFO_H
#define K_BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

union k_boot_info_t
{
    struct 
    {
        uint32_t kernel_start;
        uint32_t kernel_size;   
    };

    uint8_t bytes[512];
};


#endif