#ifndef K_DSK_DEFS_H
#define K_DSK_DEFS_H

#include <stdint.h> 
#include <stddef.h>

#include "../proc/defs.h"

// struct k_dsk_cmd_page_header_t
// {
//     struct k_dsk_cmd_page_t *next;
// };

// #define K_DSK_CMD_PAGE_CMD_COUNT ((4096 - sizeof(struct k_dsk_cmd_page_header_t)) / sizeof(struct k_dsk_cmd_t))
// struct k_dsk_cmd_page_t
// {
//     struct k_dsk_cmd_page_header_t header;
//     struct k_dsk_cmd_t cmds[K_DSK_CMD_PAGE_CMD_COUNT];
// };

// struct k_dsk_cmd_queue_t
// {
//     struct k_dsk_cmd_page_t *first;
//     struct k_dsk_cmd_page_t *last;
// };

enum K_DSK_CMD_TYPES
{
    K_DSK_CMD_TYPE_READ_START = 0,
    K_DSK_CMD_TYPE_READ_END,
    K_DSK_CMD_TYPE_WRITE_START,
    K_DSK_CMD_TYPE_WRITE_END,
    K_DSK_CMD_TYPE_TRANSFER_PART,
    K_DSK_CMD_TYPE_LAST
};

struct k_dsk_cmd_t
{
    uint64_t address : 58;
    uint64_t type : 6;
    uint32_t size;
    void *buffer;
};

struct k_dsk_cmd_page_t
{

};
struct k_dsk_cmd_buffer_t
{
    uint32_t cmd_count;
    struct k_dsk_cmd_t *cmds;
};

struct k_dsk_transaction_t
{
    uint32_t transaction_id;

};

enum K_DSK_TYPES
{
    K_DSK_TYPE_RAM = 0,
    K_DSK_TYPE_DISK,
    K_DSK_TYPE_USB,
    K_DSK_TYPE_CDROM,
    K_DSK_TYPE_BOOT
};

struct k_dsk_disk_t
{
    uint32_t type;
    uint32_t start_address;
    uint32_t block_size;
    uint32_t block_count;
    // struct k_dsk_cmd_queue_t queue;
};


#endif