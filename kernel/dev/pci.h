#ifndef K_PCI_H
#define K_PCI_H

#include <stdint.h>
#include "dev.h"

#define K_PCI_CONFIG_ADDRESS 0x0cf8
#define K_PCI_CONFIG_DATA 0x0cfc
#define K_PCI_INVALID_VENDOR_ID 0xffff
#define K_PCI_MULTI_FUNCTION_MASK 0x80
#define K_PCI_MAX_BUS_DEVICES 32
#define K_PCI_MAX_DEVICE_FUNCTIONS 8
#define K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, offset) (0x80000000 | ( (uint32_t)(bus & 0xff) << 16) | ((uint32_t)(device & 0x1f) << 11) | ((uint32_t)(function & 0x7) << 8) | ((uint32_t)offset & 0xfc))


enum K_PCI_VENDOR_IDS
{
    K_PCI_VENDOR_ID_INTEL = 0x8086
};

enum K_PCI_HEADER_TYPES
{
    K_PCI_HEADER_TYPE_BASE = 0x00,
    K_PCI_HEADER_TYPE_PCI_BRIDGE = 0x01,
    K_PCI_HEADER_TYPE_CARDBUS_BRIDGE = 0x02,
    K_PCI_HEADER_TYPE_MULTIFUN = 1 << 7
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

enum K_PCI_COMMAND_FLAGS
{
    K_PCI_COMMAND_FLAG_IO_SPACE = 1,
    K_PCI_COMMAND_FLAG_MEMORY_SPACE = 1 << 1,
    K_PCI_COMMAND_FLAG_PCI_MASTER = 1 << 2,
    K_PCI_COMMAND_FLAG_SPECIAL_CYCLES = 1 << 3,
    K_PCI_COMMAND_FLAG_WRITE_INVALIDATE_ENABLE = 1 << 4,
    K_PCI_COMMAND_FLAG_VGA_PALETTE_SNOOP = 1 << 5,
    K_PCI_COMMAND_FLAG_PARITY_ERROR_RESPONSE = 1 << 6,
    K_PCI_COMMAND_FLAG_STEPPING_CONTROL = 1 << 7,
    K_PCI_COMMAND_FLAG_SERR_ENABLE = 1 << 8,
    K_PCI_COMMAND_FLAG_FAST_BACK_TO_BACK_ENABLE = 1 << 9,
};

enum K_PCI_STATUS_FLAGS
{
    K_PCI_STATUS_FLAG_CAP_LIST = 1 << 4,
    K_PCI_STATUS_FLAG_66MHZ_CAPABLE = 1 << 5,
    K_PCI_STATUS_FLAG_FAST_BACK_TO_BACK_CAPABLE = 1 << 7,
    K_PCI_STATUS_FLAG_MASTER_DATA_PARITY_ERROR = 1 << 8,
    K_PCI_STATUS_FLAG_DEVICE_TIMING_FAST = 0,
    K_PCI_STATUS_FLAG_DEVICE_TIMING_MEDIUM = 1 << 9,
    K_PCI_STATUS_FLAG_DEVICE_TIMING_SLOW = 1 << 10,
    K_PCI_STATUS_FLAG_SIGNALED_TARGET_ABORT = 1 << 11,
    K_PCI_STATUS_FLAG_RECEIVED_TARGET_ABORT = 1 << 12,
    K_PCI_STATUS_FLAG_RECEIVED_MASTER_ABORT = 1 << 13,
    K_PCI_STATUS_FLAG_SIGNALED_MASTER_ABORT = 1 << 14,
    K_PCI_STATUS_FLAG_DETECTED_PARITY_ERROR = 1 << 15
};

#define K_PCI_STATUS_TIMING_MASK 0x0600

enum K_PCI_CONFIG_REGS
{
    K_PCI_CONFIG_REG_DEVICE_ID = 0,
    K_PCI_CONFIG_REG_VENDOR_ID,
    K_PCI_CONFIG_REG_STATUS,
    K_PCI_CONFIG_REG_COMMAND,
    K_PCI_CONFIG_REG_CLASS_CODE,
    K_PCI_CONFIG_REG_REVISION_ID,
    K_PCI_CONFIG_REG_BIST,
    K_PCI_CONFIG_REG_HEADER_TYPE,
    K_PCI_CONFIG_REG_LATENCY_TIMER,
    K_PCI_CONFIG_REG_CACHE_LINE_SIZE,
    
    K_PCI_CONFIG_REG_BAR0,
    K_PCI_CONFIG_REG_BAR1,
    K_PCI_CONFIG_REG_BAR2,
    K_PCI_CONFIG_REG_BAR3,
    K_PCI_CONFIG_REG_BAR4,
    K_PCI_CONFIG_REG_BAR5,
    K_PCI_CONFIG_REG_BAR6,
    K_PCI_CONFIG_REG_LAST
};

struct k_pci_config_reg_t
{
    uint8_t offset;
    uint8_t size;
};

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
                uint8_t capability_list;
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
enum K_PCI_CAPABILITIES
{
    K_PCI_CAPABILITY_MSI = 5
};

struct k_pci_capability_t
{
    uint8_t capability_id;
    uint8_t next;
};


enum K_PCI_MSI_CAPABILITY_FLAGS
{
    K_PCI_MSI_CAPABILITY_FLAG_ENABLE = 1,
    K_PCI_MSI_CAPABILITY_FLAG_64_CAPABLE = 1 << 7,
};

#define K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_SHIFT 0x1
#define K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_MASK 0x7
#define K_PCI_MSI_MULTIPLE_MESSAGE_ENABLE_SHIFT 0x4
#define K_PCI_MSI_MULTIPLE_MESSAGE_ENABLE_MASK 0x7

struct k_pci_msi32_capability_t
{
    uint8_t capability_id;
    uint8_t next;
    uint16_t message_control;
    uint32_t message_address;
    uint16_t message_data;
};

struct k_pci_msi64_capability_t
{
    uint8_t capability_id;
    uint8_t next;
    uint16_t message_control;
    uint32_t message_addressl;
    uint32_t message_addressh;
    uint16_t message_data;
};

#define K_PCI_MSI_MESSAGE_CONTROL_WORD (offsetof(struct k_pci_msi32_capability_t, message_control) >> 1)
#define K_PCI_MSI_MESSAGE_ADDRESSL_DWORD (offsetof(struct k_pci_msi64_capability_t, message_addressl) >> 2)
#define K_PCI_MSI_MESSAGE_ADDRESSH_DWORD (offsetof(struct k_pci_msi64_capability_t, message_addressh) >> 2)
#define K_PCI_MSI32_MESSAGE_DATA_WORD (offsetof(struct k_pci_msi32_capability_t, message_data) >> 1)
#define K_PCI_MSI64_MESSAGE_DATA_WORD (offsetof(struct k_pci_msi64_capability_t, message_data) >> 1)

struct k_pci_msi_message_t
{
    uint32_t message;
};

struct k_pci_device_t
{
    struct k_dev_device_t base;

    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t interrupt_pin;

    uint32_t message_count;
    struct k_pci_msi_message_t *messages;

    uint32_t bar_count;
    uint32_t *bars;
};

struct k_pci_func_info_t
{
    char *              name;
    uint8_t             index;
    uint32_t          (*init_func)(uint8_t bus_index, uint8_t device_index);
};

struct k_pci_device_info_t
{
    char *name;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t (*init)(uint8_t bus_index, uint8_t device_index, union k_pci_header_t *header);
    struct k_pci_func_info_t *funcs;
};

struct k_pci_init_info_t
{
    union k_pci_header_t *header;
    struct k_pci_device_info_t *device;
    struct k_pci_func_info_t *function;
    // struct k_pci_id_info_t *info;
};

void k_pci_Init();

uint32_t k_pci_SetupDevice(union k_pci_header_t *header, struct k_pci_device_info_t *info, uint8_t bus, uint8_t device);

uint32_t k_pci_ReadHeader(uint8_t bus, uint8_t device, uint8_t function, union k_pci_header_t *header);

uint32_t k_pci_ReadDword(uint32_t base_address, uint32_t offset);

void k_pci_WriteDword(uint32_t base_address, uint32_t dword, uint32_t value);

uint16_t k_pci_ReadWord(uint32_t base_address, uint32_t offset);

void k_pci_WriteWord(uint32_t base_address, uint32_t offset, uint16_t value);

// uint32_t k_pci_read_config_reg(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg);

// void k_pci_write_config_reg(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t value);

// void k_pci_refresh_device(struct k_pci_device_t *device);

// struct k_dev_device_t *k_pci_discover_device(uint8_t bus, uint8_t device);



#endif