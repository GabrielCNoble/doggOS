#include "k_dev.h"
#include "mem/k_mem.h"
#include "k_pci.h"
#include "k_term.h"
#include <stddef.h>

// uint8_t *k_device_mem = NULL;
// uint32_t k_device_offset = 0;

struct k_dev_device_t *k_dev_devices = NULL;
uint32_t k_dev_device_count = 0;



void k_dev_init()
{
    // k_dev_devices = k_mem_alloc(sizeof(struct k_dev_device_t) * K_DEV_MAX_DEVICES, sizeof(struct k_dev_device_t));
    k_dev_enumerate_pci(0);
}

void k_dev_enumerate_pci(uint32_t bus)
{
    for(uint32_t device_index = 0; device_index < K_PCI_MAX_BUS_DEVICES; device_index++)
    {
        union k_pci_header_t header = {};
        uint32_t function_count = 1;
        k_pci_read_header(bus, device_index, 0, &header);

        if(header.vendor_id != K_PCI_INVALID_VENDOR_ID)
        {
            if(header.header_type & K_PCI_HEADER_TYPE_MULTIFUN)
            {
                function_count = K_PCI_MAX_DEVICE_FUNCTIONS;
                k_printf("======= device %x:%x is multi function ======= \n", (uint32_t)header.vendor_id, (uint32_t)header.device_id);
            }
            else
            {
                k_printf("======= device %x:%x is single function ======= \n", (uint32_t)header.vendor_id, (uint32_t)header.device_id);
            }

            for(uint32_t function_index = 0; function_index < function_count; function_index++)
            {
                if(header.vendor_id != K_PCI_INVALID_VENDOR_ID)
                {
                    k_printf("function %d: 0x%x - 0x%x\n", function_index, header.device_class, header.device_subclass);

                    // if(header.status & K_PCI_STATUS_FLAG_CAP_LIST)
                    // {
                    //     struct k_pci_capability_t *capability = (struct k_pci_capability_t *)(header.dwords + (header.type0.cap_pointer >> 2));

                    //     do
                    //     {
                    //         if(capability->capability_id == K_PCI_CAPABILITY_MSI)
                    //         {
                    //             struct k_pci_msi_capability32_t *msi_capability = (struct k_pci_msi_capability32_t *)capability;
                    //             uint32_t required_messages = (msi_capability->message_control >> K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_SHIFT) & K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_MASK;
                    //             uint32_t address_width = 32;

                    //             if(msi_capability->message_control & K_PCI_MSI_CAPABILITY_FLAG_64_CAPABLE)
                    //             {
                    //                 struct k_pci_msi_capability64_t *msi64_capability = (struct k_pci_msi_capability64_t *)capability;
                    //                 address_width = 64;
                    //             }

                    //             k_printf("msi supported - messages: %d - %d bit message address\n", 1 << required_messages, address_width);
                    //         }
                    //         capability = (struct k_pci_capability_t *)(header.dwords + (capability->next >> 2));
                    //     }
                    //     while(capability->next);
                    // }
                }

                k_pci_read_header(bus, device_index, function_index + 1, &header);
            }
        }
    }
}

// struct k_device_t *k_dev_alloc_device(uint32_t size)
// {
    // struct k_device_t *device = NULL;

    // if(k_device_offset + size < K_DEV_MEM_SIZE)
    // {
    //     device = (struct k_device_t *)(k_device_mem + k_device_offset);
    //     k_device_offset = (k_device_offset + size - 3) & (~3);
        
    //     device->next = NULL;
    //     device->prev = NULL;

    //     if(!k_devices)
    //     {
    //         k_devices = device;
    //     }
    //     else
    //     {
    //         k_last_device->next = device;
    //         device->prev = k_last_device;
    //     }

    //     k_last_device = device;
    // }

    // return device;
// }

// void k_dev_disk_read(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count)
// {

// }

// void k_dev_disk_write(struct k_disk_device_t *device, void *buffer, uint32_t buffer_size, uint32_t address, uint32_t count)
// {

// }

