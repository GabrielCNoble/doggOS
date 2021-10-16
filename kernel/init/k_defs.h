#ifndef K_INIT_DEFS_H
#define K_INIT_DEFS_H

struct k_init_data_t
{
    struct k_mem_range_t *ranges;
    uint32_t range_count;
    uint32_t boot_drive;
};

#endif