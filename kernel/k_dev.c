#include "k_dev.h"
#include "k_mem.h"
#include <stddef.h>

uint8_t *k_device_mem = NULL;
uint32_t k_device_offset = 0;

struct k_device_t *k_devices = NULL;
struct k_device_t *k_last_device = NULL;

void k_dev_init()
{
    /* contiguous block to avoid the risk of having devices scattered around the heap */
    k_device_mem = k_mem_alloc(K_DEV_MEM_SIZE);
}

struct k_device_t *k_dev_alloc_device(uint32_t size)
{
    struct k_device_t *device = NULL;

    if(k_device_offset + size < K_DEV_MEM_SIZE)
    {
        device = (struct k_device_t *)(k_device_mem + k_device_offset);
        k_device_offset = (k_device_offset + size - 3) & (~3);
        
        device->next = NULL;
        device->prev = NULL;

        if(!k_devices)
        {
            k_devices = device;
        }
        else
        {
            k_last_device->next = device;
            device->prev = k_last_device;
        }

        k_last_device = device;
    }

    return device;
}

void k_dev_disk_read(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count)
{

}

void k_dev_disk_write(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count)
{

}

