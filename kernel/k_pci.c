#include "k_pci.h"
#include "k_io.h"
#include "k_term.h"


uint32_t k_device_count = 0;
struct k_pci_device_t k_devices[64];


void k_pci_init()
{
    k_pci_enumerate();
}

uint32_t k_pci_read_header(uint8_t bus, uint8_t device, uint8_t function, struct k_pci_header_t *header)
{
    uint32_t config_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, 0x0);
    k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
    uint32_t data = k_inportd(K_PCI_CONFIG_DATA);

    if((data & 0xffff) == 0xffff)
    {
        return 0;
    }

    header->vendor_id = data & 0xffff;
    header->device_id = (data >> 16) & 0xffff;

    config_address += 0x4;
    k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
    data = k_inportd(K_PCI_CONFIG_DATA);
    header->command = data & 0xffff;
    header->status = (data >> 16) & 0xffff;

    config_address += 0x4;
    k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
    data = k_inportd(K_PCI_CONFIG_DATA);
    header->revision_id = data & 0xff;
    header->device_subclass = (data >> 16) & 0xff; 
    header->device_class = (data >> 24) & 0xff;
    
    config_address += 0x4;
    k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
    data = k_inportd(K_PCI_CONFIG_DATA);
    header->cacheline_size = data & 0xff;
    header->latency_timer = (data >> 8) & 0xff;
    header->header_type = (data >> 16) & 0xff;
    header->bist = (data >> 24) & 0xff;

    switch(header->header_type)
    {
        case K_PCI_HEADER_TYPE_BASE:
            for(uint32_t reg_index = 0; reg_index < 6; reg_index++)
            {
                config_address += 0x4;
                k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
                header->base_address_regs0[reg_index] = k_inportd(K_PCI_CONFIG_DATA);
            }

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            header->cardbus_cis_pointer = k_inportd(K_PCI_CONFIG_DATA);
        break;

        case K_PCI_HEADER_TYPE_PCI_BRIDGE:
            for(uint32_t reg_index = 0; reg_index < 2; reg_index++)
            {
                config_address += 0x4;
                k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
                header->base_address_regs1[reg_index] = k_inportd(K_PCI_CONFIG_DATA);
            }

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->primary_bus_number = data & 0xff;
            header->secondary_bus_number = (data >> 8) & 0xff;
            header->subordinate_bus_number = (data >> 16) & 0xff;
            header->secondary_latency_timer = (data >> 24) & 0xff;

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->io_base = data & 0xff;
            header->io_limit = (data >> 8) & 0xff;
            header->secondary_status = (data >> 16) & 0xffff;

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->memory_base = data & 0xffff;
            header->memory_limit = (data >> 16) & 0xffff;

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->prefetchable_memory_base = data & 0xffff;
            header->prefetchable_memory_limit = (data >> 16) & 0xffff;

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            header->prefetchable_memory_base_hdword = k_inportd(K_PCI_CONFIG_DATA);

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            header->prefetchable_memory_limit_hdword = k_inportd(K_PCI_CONFIG_DATA);

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->io_base_hword = data & 0xffff;
            header->io_limit_hword = (data >> 16) & 0xffff;

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->cap_pointer1 = data & 0xff;
            
            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            header->exp_rom_base_address = k_inportd(K_PCI_CONFIG_DATA);

            config_address += 0x4;
            k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
            data = k_inportd(K_PCI_CONFIG_DATA);
            header->interrupt_line = data & 0xff;
            header->interrupt_pin1 = (data >> 8) & 0xff;
            header->bridge_control = (data >> 16) & 0xffff;
        break;
    }

    return 1;
}

void k_pci_discover_devices(uint8_t bus)
{
    for(uint32_t device_index = 0; device_index < 32; device_index++)
    {
        struct k_pci_device_t *device = k_devices + k_device_count;

        if(k_pci_read_header(bus, device_index, 0, &device->header))
        {
            device->config_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device_index, 0, 0);
            k_device_count++;

            k_printf("vendor id: 0x%x | device id: 0x%x \n", (uint32_t)device->header.vendor_id, (uint32_t)device->header.device_id);

            if(device->header.header_type & 0x80)
            {
                k_printf("device is multi function\n");
                k_printf("function 0: class: 0x%x | subclass: 0x%x\n", (uint32_t)device->header.device_class, (uint32_t)device->header.device_subclass);
                for(uint32_t function_index = 1; function_index < 8; function_index++)
                {
                    device = k_devices + k_device_count;
                    if(k_pci_read_header(bus, device_index, function_index, &device->header))
                    {
                        k_device_count++;
                        k_printf("function %d: class: 0x%x | subclass: 0x%x\n", function_index, (uint32_t)device->header.device_class, (uint32_t)device->header.device_subclass);
                    }
                }
            }
            else
            {
                k_printf("device is single function\n");
                k_printf("class: 0x%x | subclass: 0x%x\n", (uint32_t)device->header.device_class, (uint32_t)device->header.device_subclass);
            }
            k_puts("\n");
        }
    }
}

void k_pci_enumerate()
{
    k_pci_discover_devices(0);
}