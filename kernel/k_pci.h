#ifndef K_PCI_H
#define K_PCI_H

#include <stdint.h>

#define K_PCI_CONFIG_ADDRESS 0x0cf8
#define K_PCI_CONFIG_DATA 0x0cfc
#define K_PCI_INVALID_VENDOR_ID 0xffff
#define K_PCI_MULTI_FUNCTION_MASK 0x80
#define K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, offset) (0x80000000 | ( (uint32_t)(bus & 0xff) << 16) | ((uint32_t)(device & 0x1f) << 11) | ((uint32_t)(function & 0x7) << 8) | ((uint32_t)offset & 0xfc))


enum K_PCI_HEADER_TYPES
{
    K_PCI_HEADER_TYPE_BASE = 0x00,
    K_PCI_HEADER_TYPE_PCI_BRIDGE = 0x01,
    K_PCI_HEADER_TYPE_CARDBUS_BRIDGE = 0x02
};


enum K_PCI_DEVICE_CLASS
{
    K_PCI_DEVICE_CLASS_MASS_STORAGE_CONTROLLER = 0x01,
    K_PCI_DEVICE_CLASS_NETWORK_CONTROLLER = 0x02,
    K_PCI_DEVICE_CLASS_DISPLAY_CONTROLLER = 0x03,
    K_PCI_DEVICE_CLASS_MULTIMEDIA_DEVICE = 0x04,
    K_PCI_DEVICE_CLASS_MEMORY_CONTROLLER = 0x05,
    K_PCI_DEVICE_CLASS_BRIDGE_DEVICE = 0x06,
    K_PCI_DEVICE_CLASS_SIMPLE_COMM_CONTROLLER = 0x07,
    K_PCI_DEVICE_CLASS_BASE_SYSTEM_PERIPHERALS = 0x08,
    K_PCI_DEVICE_CLASS_INPUT_DEVICE = 0x09,
    K_PCI_DEVICE_CLASS_DOCKING_STATION = 0x0a,
    K_PCI_DEVICE_CLASS_PROCESSORS = 0x0b,
    K_PCI_DEVICE_CLASS_SERIAL_BUS_CONTROLLER = 0x0c,
    K_PCI_DEVICE_CLASS_WIRELESS_CONTROLLER = 0x0d,
    K_PCI_DEVICE_CLASS_INTELLIGENT_IO_CONTROLLER = 0x0e,
    K_PCI_DEVICE_CLASS_SATELLITE_COMM_CONTROLLER = 0x0f,
    K_PCI_DEVICE_CLASS_ENCRYPT_DECRYPT_CONTROLLER = 0x10,
    K_PCI_DEVICE_CLASS_DONT_FIT = 0xff,
};

enum K_PCI_MASS_STORAGE_DEVICE_SUBCLASS
{
    K_PCI_MASS_STORAGE_DEVICE_SUBCLASS_IDE_CONTROLLER = 0x01,
    K_PCI_MASS_STORAGE_DEVICE_SUBCLASS_FLOPPY_CONTROLLER = 0x02,
    K_PCI_MASS_STORAGE_DEVICE_SUBCLASS_RAID_CONTROLLER = 0x04,
    K_PCI_MASS_STORAGE_DEVICE_SUBCLASS_AHCI_CONTROLLER = 0x06,
};

// enum K_PCI_DEVICE_SUBCLASS
// {

// };

// enum K_PCI_COMMAND_BITS
// {
//     // K_PCI_COMMAND_BIT_
// };

union k_pci_header_t
{
    struct
    {
        uint16_t vendor_id; 
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t reserved0;
        uint8_t device_subclass;
        uint8_t device_class;
        uint8_t cacheline_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t bist;

        union
        {
            /* type 0 header */
            struct
            {
                uint32_t base_address_regs[6];
                uint32_t cardbus_cis_pointer;
                uint16_t subsystem_vendor_id;
                uint16_t subsystem_id;
                uint32_t exp_rom_base_address;
                uint8_t reserved1[3];
                uint8_t cap_pointer;
                uint32_t reserved2;
                uint8_t max_lat;
                uint8_t min_gnt;
                uint8_t interrupt_pin;
                uint8_t interrupt_line;
            } type0;

            /* type 1 header */
            struct
            {
                uint32_t base_address_regs[2];
                uint8_t primary_bus_number;
                uint8_t secondary_bus_number;
                uint8_t subordinate_bus_number;
                uint8_t secondary_latency_timer;
                uint8_t io_base;
                uint8_t io_limit;
                uint16_t secondary_status;
                uint16_t memory_base;
                uint16_t memory_limit;
                uint16_t prefetchable_memory_base;
                uint16_t prefetchable_memory_limit;
                uint32_t prefetchable_memory_base_hdword;
                uint32_t prefetchable_memory_limit_hdword;
                uint16_t io_base_hword;
                uint16_t io_limit_hword;
                uint8_t cap_pointer;
                uint8_t reserved4[3];
                uint32_t expansion_rom_base_address;
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint16_t bridge_control;
            } type1;

            /* type 2 header */
            struct
            {

            };
        };
    };

    uint32_t dwords[64];
};

struct k_pci_function_t
{
    union k_pci_header_t header;
    uint32_t function_number;
};

struct k_pci_device_t
{
    struct k_pci_function_t *functions;
    uint32_t config_address;
    uint32_t function_count;
};

void k_pci_init();

uint32_t k_pci_read_header(uint8_t bus, uint8_t device, uint8_t function, union k_pci_header_t *header);

void k_pci_discover_devices(uint8_t bus);

void k_pci_enumerate();



#endif