#include "k_pci.h"
#include "k_io.h"
#include "k_term.h"


void k_pci_enumerate()
{
    k_pci_discover_devices(0);
}

uint32_t k_pci_read_header(uint8_t bus, uint8_t device, uint8_t function, union k_pci_header_t *header)
{
    uint32_t config_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, 0x0);
    k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
    header->dwords[0] = k_inportd(K_PCI_CONFIG_DATA);

    if((header->dwords[0] & 0xffff) == 0xffff)
    {
        return 0;
    }

    for(uint32_t dword_index = 1; dword_index < 64; dword_index++)
    {
        config_address += 0x4;
        k_outportd(config_address, K_PCI_CONFIG_ADDRESS);
        header->dwords[dword_index] = k_inportd(K_PCI_CONFIG_DATA);
    }

    return 1;
}

void k_pci_discover_devices(uint8_t bus)
{
    // for(uint32_t device_index = 0; device_index < 32; device_index++)
    // {
    //     struct k_pci_device_t *device = k_devices + k_device_count;
    //     struct k_pci_function_t *function = k_functions + k_function_count;
    //     if(k_pci_read_header(bus, device_index, 0, &function->header))
    //     {
    //         uint32_t function_count = 1;
    //         device->config_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device_index, 0, 0);
    //         device->functions = function;
    //         function->function_number = 0;
    //         k_device_count++;
    //         k_function_count++;

    //         if(function->header.header_type & 0x80)
    //         {   
    //             for(uint32_t function_index = 1; function_index < 8; function_index++)
    //             {
    //                 function = k_functions + k_function_count;
    //                 if(k_pci_read_header(bus, device_index, function_index, &function->header))
    //                 {
    //                     function->function_number = function_index;
    //                     k_function_count++;
    //                     function_count++;
    //                 }
    //             }
    //         }

    //         device->function_count = function_count;

    //         for(uint32_t function_index = 0; function_index < device->function_count; function_index++)
    //         {
    //             function = device->functions + function_index;
    //             union k_pci_header_t *header = &function->header;
    //             k_printf("index: %d | ven: 0x%x | dev: 0x%x \n", device_index, (uint32_t)header->vendor_id, (uint32_t)header->device_id);
    //             k_printf("fn: %d | c: 0x%x | sc: 0x%x | iline: 0x%x | ipin: 0x%x\n", (uint32_t)function->function_number, 
    //                                                                                  (uint32_t)header->device_class, 
    //                                                                                  (uint32_t)header->device_subclass, 
    //                                                                                  (uint32_t)header->type0.interrupt_line,
    //                                                                                  (uint32_t)header->type0.interrupt_pin);

    //             k_printf("ba0: 0x%x | ba1: 0x%x | ba2: 0x%x | ba3: 0x%x | ba4: 0x%x | ba5: 0x%x\n", function->header.type0.base_address_regs[0],
    //                                                             function->header.type0.base_address_regs[1],
    //                                                             function->header.type0.base_address_regs[2],
    //                                                             function->header.type0.base_address_regs[3],
    //                                                             function->header.type0.base_address_regs[4],
    //                                                             function->header.type0.base_address_regs[5]);


                
    //         }

    //         k_puts("\n");
    //     }
    // }
}