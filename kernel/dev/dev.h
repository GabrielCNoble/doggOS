#ifndef K_DEV_H
#define K_DEV_H

#include <stdint.h>
#include "../proc/thread.h"
#include "../rt/atm.h"

enum K_DEV_DEVICE_TYPE
{
    K_DEV_DEVICE_TYPE_PCI,
    K_DEV_DEVICE_TYPE_DISK,
    K_DEV_DEVICE_TYPE_KEYBOARD,
    K_DEV_DEVICE_TYPE_MOUSE,
    K_DEV_DEVICE_TYPE_TIMER,
    K_DEV_DEVICE_TYPE_NETWORK,
    K_DEV_DEVICE_TYPE_LAST,
};

enum K_DEV_DEVICE_STATUS
{
    K_DEV_DEVICE_STATUS_READY = 0,
    K_DEV_DEVICE_STATUS_NOT_INITIALIZED,
    K_DEV_DEVICE_STATUS_INITIALIZING,
    K_DEV_DEVICE_STATUS_INIT_FAILED,
};

/* forward declaration */
struct k_dev_device_t;

typedef uint32_t (*k_dev_func_t)(struct k_dev_device_t *device);

#define K_DEVICE_DESC_FIELDS                    \
    uint32_t                device_type;        \
    size_t                  device_size;        \
    k_proc_thread_func_t    driver_func;        \
    const char              name[32];           \

struct k_dev_device_desc_t
{
    K_DEVICE_DESC_FIELDS;
};

#define K_DEVICE_FIELDS                                                                                     \
    uint32_t                        device_type;                                                            \
    uint32_t                        device_status;                                                          \
    struct k_dev_device_t *         next_device;                                                            \
    struct k_dev_device_t *         prev_device;                                                            \
    /* this will be part of the driver binary */                                                            \
    k_proc_thread_func_t            driver_func;                                                            \
    /* this will become a process */                                                                        \
    struct k_proc_thread_t *        driver_thread;                                                          \
    k_rt_cond_t                     device_cond;                                                            \
    const char                      name[32];                                                               \

struct k_dev_device_t 
{
    K_DEVICE_FIELDS;
};

void k_dev_Init();

struct k_dev_device_t *k_dev_CreateDevice(struct k_dev_device_desc_t *device_desc);

void k_dev_StartDevices();

void k_dev_StartDevice(struct k_dev_device_t *device);

void k_dev_WaitDeviceInit(struct k_dev_device_t *device);

void k_dev_ShutdownDevice(struct k_dev_device_t *device);

void k_dev_DestroyDevice(struct k_dev_device_t *device);

void k_dev_DeviceInitFailed(struct k_dev_device_t *device);

void k_dev_DeviceReady(struct k_dev_device_t *device);

// void k_dev_RegisterDevice(k_dev_init_func_t init_func, void *init_info);

// void k_dev_enumerate_pci(uint32_t bus);

#endif