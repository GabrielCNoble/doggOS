#ifndef K_DEV_H
#define K_DEV_H

#include <stdint.h>

/* 128KB for devices. Some may require additional space to store required data structures.
This additional space will be allocated by the code responsible to control the device. For
example, ahci disk controllers require a bunch of space to store command headers, tables
and prd entries. Space for this will be allocated and properly devided by k_ahci.c  */
#define K_DEV_MEM_SIZE 0x1fffe

enum K_DEVICE_TYPE
{
    K_DEVICE_TYPE_NONE = 0,
    K_DEVICE_TYPE_DISK_CONTROLLER = 1,
    K_DEVICE_TYPE_NETWORK_ADAPTER = 2,
};

struct k_device_t
{
    struct k_device_t *next;
    struct k_device_t *prev;
    uint32_t type;
};

/* generic disk device. As is this is just a dummy device, that does nothing.
Depending on the actual device, more fields may be present at the end of the 
struct, and those fields will be used by the appropriate read/write functions */
// struct k_disk_device_t
// {
//     struct k_device_t *next;
//     struct k_device_t *prev;
//     uint32_t type;

//     void (*read)(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count);
//     void (*write)(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count);
// };

void k_dev_init();

struct k_device_t *k_dev_alloc_device(uint32_t size);

// void k_dev_disk_read(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count);

// void k_dev_disk_write(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count);


#endif