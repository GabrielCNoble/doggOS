#ifndef K_DEV_DSK_H
#define K_DEV_DSK_H

#include "dev.h"
#include "../proc/defs.h"
#include "../rt/atm.h" 

enum K_DEV_DSK_CMD_TYPES
{
    K_DEV_DSK_CMD_TYPE_READ,
    K_DEV_DSK_CMD_TYPE_WRITE,
    K_DEV_DSK_CMD_TYPE_CLEAR,
    K_DEV_DSK_CMD_TYPE_IDENTIFY,
    K_DEV_DSK_CMD_TYPE_LAST
};

struct k_dev_dsk_cmd_t
{
    struct k_dev_dsk_cmd_t *next;
    k_rt_cond_t         condition;
    uint32_t            address;
    uint32_t            size;
    uint32_t            type : 16;
    uint32_t            status: 16;
    void               *buffer;
};

// struct k_dev_dsk_cmd_page_t
// {

// };

// struct k_dev_dsk_cmd_buffer_t
// {
//     uint32_t cmd_count;
//     struct k_dsk_cmd_t *cmds;
// };

// struct k_dsk_transaction_t
// {
//     uint32_t transaction_id;

// };

enum K_DEV_DSK_TYPES
{
    K_DEV_DSK_TYPE_RAM = 0,
    K_DEV_DSK_TYPE_DISK,
    K_DEV_DSK_TYPE_USB,
    K_DEV_DSK_TYPE_CDROM,
    K_DEV_DSK_TYPE_BOOT,
    K_DEV_DSK_TYPE_LAST
};

struct k_dev_disk_t;

typedef uint32_t (*k_dev_disk_func_t)(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd);

// struct k_dev_disk_def_t 
// {
//     K_DEVICE_DESC_FIELDS;

//     k_dev_disk_func_t       read;
//     k_dev_disk_func_t       write;
//     k_dev_disk_func_t       clear;

//     uint32_t                start_address;
//     uint32_t                block_size;
//     uint32_t                block_count;
//     uint32_t                type;

//     /* disk specific data */
//     void *                  data;
// };

struct k_dev_disk_t
{
    K_DEVICE_FIELDS;
    struct k_dev_disk_t *           next;
    struct k_dev_dsk_cmd_t *        cmds;
    struct k_dev_dsk_cmd_t *        last_cmd;

    k_dev_disk_func_t       read;
    k_dev_disk_func_t       write;
    k_dev_disk_func_t       clear;

    uint64_t                start_address;
    uint32_t                block_size;
    uint32_t                block_count;
    uint32_t                type;

    /* disk specific data */
    void *                  data;

    // struct k_dev_disk_def_t         def;
};

// struct k_dev_disk_t *k_dev_CreateDisk(struct k_dev_disk_def_t *disk_def);

struct k_dev_dsk_cmd_t *k_dev_AllocDiskCmd();

void k_dev_FreeDiskCmd(struct k_dev_dsk_cmd_t *cmd);

void k_dev_EnqueueDiskCmd(struct k_dev_disk_t *disk, struct k_dev_dsk_cmd_t *cmd);

uint32_t k_dev_DiskRead(struct k_dev_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dev_DiskWrite(struct k_dev_disk_t *disk, uint32_t start, uint32_t count, void *data);

uint32_t k_dev_DiskClear(struct k_dev_disk_t *disk, uint32_t start, uint32_t count);

uintptr_t k_dev_DiskThread(void *data);


#endif