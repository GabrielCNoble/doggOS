#include "boot.h"
#include "../../rt/mem.h"

extern stdcall uint32_t k_BOOT_Read_a(uint64_t first_block, uint32_t block_count, void *buffer);

uint32_t k_BOOT_Init()
{

}

uint32_t k_BOOT_Read(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd, union k_boot_info_t *boot_info)
{
    k_BOOT_Read_a(4, 1, boot_info->bytes);
}