#ifndef K_DEV_H
#define K_DEV_H

#include <stdint.h>

enum K_DEV_DEVICE_TYPE
{
    K_DEVICE_TYPE_NONE = 0,
    K_DEVICE_TYPE_PCI,
    K_DEVICE_TYPE_USB,
    K_DEVICE_TYPE_BLUETOOTH
    // K_DEVICE_TYPE_DISK_CONTROLLER = 1,
    // K_DEVICE_TYPE_NETWORK_ADAPTER = 2,
};

struct k_dev_device_t
{
    uint32_t type;
    struct k_dev_device_t *next;
};

// struct k_dev_device_funcs_t
// {
//     uint32_t (*init)();
//     uint32_t (*reset)();
// };

typedef uint32_t (*k_dev_init_func_t)(void *init_info);
struct k_dev_driver_t
{
    k_dev_init_func_t init;
    // void *id_info;
    // uint32_t (*init)(void *init_info);
    // uint32_t (*reset)();
    // struct k_dev_device_funcs_t funcs;
};

void k_dev_Init();

void k_dev_RegisterDevice(k_dev_init_func_t init_func, void *init_info);

// void k_dev_enumerate_pci(uint32_t bus);

#endif