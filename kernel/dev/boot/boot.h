#ifndef K_BOOT_H
#define K_BOOT_H

#include "../dev.h"
#include "../dsk.h"
#include "../../init/defs.h"
#include "../../boot/k_bootinfo.h"

struct k_boot_disk_packet_t
{
    uint8_t             packet_size;
    uint8_t             reserved;
    uint16_t            block_count;
    union
    {
        uint32_t        buffer;
        struct
        {
            uint16_t    offset;
            uint16_t    segment;
        };
    };
    uint64_t            start_lba;
};

uint32_t k_BOOT_Init();

uint32_t k_BOOT_Read(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd, union k_boot_info_t *boot_info);




#endif