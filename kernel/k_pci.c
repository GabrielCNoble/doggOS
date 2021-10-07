#include "k_pci.h"
#include "k_cpu.h"
#include "k_term.h"
#include "mem/k_mem.h"
#include <stddef.h>

struct k_pci_config_reg_t k_pci_config_regs[] =
{
    [K_PCI_CONFIG_REG_DEVICE_ID] = {.offset = 0x00, .size = 2}, 
    [K_PCI_CONFIG_REG_VENDOR_ID] = {.offset = 0x02, .size = 2},
    [K_PCI_CONFIG_REG_STATUS] = {.offset = 0x04, .size = 2},
    [K_PCI_CONFIG_REG_COMMAND] = {.offset = 0x06, .size = 2},
    [K_PCI_CONFIG_REG_CLASS_CODE] = {.offset = 0x08, .size = 3},
    [K_PCI_CONFIG_REG_REVISION_ID] = {.offset = 0x0b, .size = 1},
    [K_PCI_CONFIG_REG_BIST] = {.offset = 0x0c, .size = 1},
    [K_PCI_CONFIG_REG_HEADER_TYPE] = {.offset = 0x0d, .size = 1},
    [K_PCI_CONFIG_REG_LATENCY_TIMER] = {.offset = 0x0e, .size = 1},
    [K_PCI_CONFIG_REG_CACHE_LINE_SIZE] = {.offset = 0x0f, .size = 1},
    [K_PCI_CONFIG_REG_BAR0] = {.offset = 0x10, .size = 4},
    [K_PCI_CONFIG_REG_BAR1] = {.offset = 0x14, .size = 4},
    [K_PCI_CONFIG_REG_BAR2] = {.offset = 0x18, .size = 4},
    [K_PCI_CONFIG_REG_BAR3] = {.offset = 0x1c, .size = 4},
    [K_PCI_CONFIG_REG_BAR4] = {.offset = 0x20, .size = 4},
    [K_PCI_CONFIG_REG_BAR5] = {.offset = 0x24, .size = 4}
};

uint32_t k_pci_config_reg_masks[] = 
{
    0x00000000,
    0x000000ff,
    0x0000ffff,
    0x00ffffff,
    0xffffffff
};

uint32_t k_pci_read_header(uint8_t bus, uint8_t device, uint8_t function, union k_pci_header_t *header)
{
    uint32_t config_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, 0x0);
    k_cpu_OutD(config_address, K_PCI_CONFIG_ADDRESS);
    header->dwords[0] = k_cpu_InD(K_PCI_CONFIG_DATA);

    if(header->vendor_id == K_PCI_INVALID_VENDOR_ID)
    {
        return 0;
    }

    for(uint32_t dword_index = 1; dword_index < 64; dword_index++)
    {
        config_address += 0x4;
        k_cpu_OutD(config_address, K_PCI_CONFIG_ADDRESS);
        header->dwords[dword_index] = k_cpu_InD(K_PCI_CONFIG_DATA);
    }

    return 1;
}

uint32_t k_pci_read_dword(uint32_t base_address, uint32_t dword)
{
    base_address += dword << 2;
    k_cpu_OutD(base_address, K_PCI_CONFIG_ADDRESS);
    return k_cpu_InD(K_PCI_CONFIG_DATA);
}

void k_pci_write_dword(uint32_t base_address, uint32_t dword, uint32_t value)
{
    base_address += dword << 2;
    k_cpu_OutD(base_address, K_PCI_CONFIG_ADDRESS);
    k_cpu_OutD(value, K_PCI_CONFIG_DATA);
}

uint16_t k_pci_read_word(uint32_t base_address, uint32_t word)
{
    uint32_t value = k_pci_read_dword(base_address, word >> 1);    
    uint32_t shift = (word & 1) << 4;
    return (uint16_t)(value >> shift);
}

void k_pci_write_word(uint32_t base_address, uint32_t word, uint32_t value)
{
    uint32_t shift = (word & 1) << 4;
    uint32_t mask = 0xffff0000 >> shift;
    word >>= 1;
    uint32_t read_value = k_pci_read_dword(base_address, word) & mask;
    read_value |= value << shift;
    k_pci_write_dword(base_address, word, read_value);
}

// uint32_t k_pci_read_config_reg(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg)
// {
//     if(reg < K_PCI_CONFIG_REG_LAST)
//     {
//         struct k_pci_config_reg_t *config_reg = k_pci_config_regs + reg;

//         /* this will move the mask bits to the appropriate place inside the dword. If reg is a byte register,
//         and it starts at offset 0, this won't move any bits. If it's a byte register, and starts at offset 3
//         inside the dword, this will give an shift value of 24, which will but all the bits in the last byte 
//         of the dword. Dword regs will always be aligned to dword boundaries, so this shift value will always
//         be 0 in this case */
//         uint32_t value_shift = (config_reg->offset & 0x3) << 4;
//         uint32_t device_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, config_reg->offset >> 2);
//         k_outportd(device_address, K_PCI_CONFIG_ADDRESS);
//         uint32_t value = k_inportd(K_PCI_CONFIG_DATA) >> value_shift;
//         return value & k_pci_config_reg_masks[config_reg->size];
//     }
// }

// void k_pci_write_config_reg(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t value)
// {
//     if(reg < K_PCI_CONFIG_REG_LAST)
//     {
//         struct k_pci_config_reg_t *config_reg = k_pci_config_regs + reg;

//         /* this will move the mask bits to the appropriate place inside the dword. If reg is a byte register,
//         and it starts at offset 0, this won't move any bits. If it's a byte register, and starts at offset 3
//         inside the dword, this will give an shift value of 24, which will but all the bits in the last byte 
//         of the dword. Dword regs will always be aligned to dword boundaries, so this shift value will always
//         be 0 in this case */
//         uint32_t shift = (config_reg->offset & 0x3) << 4;
//         uint32_t mask = k_pci_config_reg_masks[config_reg->size] << shift;
//         uint32_t device_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function, config_reg->offset >> 2);
//         k_outportd(device_address, K_PCI_CONFIG_ADDRESS);
//         uint32_t dword_value = k_inportd(K_PCI_CONFIG_DATA) & (~mask);
//         dword_value |= (value << shift) & mask;
//         k_outportd(dword_value, K_PCI_CONFIG_DATA);
//     }
// }

// void k_pci_refresh_device(struct k_pci_device_t *device)
// {
//     uint32_t dword = k_pci_read_dword(device->bus, device->device, device->function, 2);
//     // device->command = dword & 0xffff;
//     // device->status = dword >> 16;
// }

struct k_dev_device_t *k_pci_discover_device(uint8_t bus, uint8_t device)
{
    union k_pci_header_t header = {};
    uint32_t function_count = 1;
    struct k_pci_device_t *pci_devices = NULL;

    if(k_pci_read_header(bus, device, 0, &header))
    {
        if(header.header_type & K_PCI_HEADER_TYPE_MULTIFUN)
        {
            function_count = K_PCI_MAX_DEVICE_FUNCTIONS;
        }

        for(uint32_t function_index = 0; function_index < function_count; function_index++)
        {
            if(header.vendor_id != K_PCI_INVALID_VENDOR_ID)
            {
                uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, function_index, 0);

                // struct k_pci_device_t *new_device = k_mem_Alloc(sizeof(struct k_pci_device_t), 0);
                // new_device->base.type = K_DEVICE_TYPE_PCI;
                // new_device->base.next = NULL;

                // new_device->bus = bus;
                // new_device->device = device;
                // new_device->function = function_index;

                uint32_t header_type = header.header_type & (~K_PCI_HEADER_TYPE_MULTIFUN);

                if(header_type == K_PCI_HEADER_TYPE_BASE)
                {
                    if(header.status & K_PCI_STATUS_FLAG_CAP_LIST)
                    {
                        uint32_t dword_offset = header.type0.cap_pointer >> 2;
                        uint32_t word_offset = dword_offset << 1;
                        struct k_pci_capability_t *capability = (struct k_pci_capability_t *)(header.dwords + dword_offset);

                        // do
                        // {
                        //     switch(capability->capability_id)
                        //     {
                        //         case K_PCI_CAPABILITY_MSI:
                        //         {
                        //             struct k_pci_msi32_capability_t *msi_32 = (struct k_pci_msi32_capability_t *)capability;

                        //             /*
                        //                 the capable amount of messages is encoded in bits 3-1 as follows:
                        //                 000 - 1 message
                        //                 001 - 2 messages
                        //                 010 - 4 messages
                        //                 011 - 8 messages
                        //                 100 - 16 messages
                        //                 101 - 32 messages
                        //                 110 - reserved
                        //                 111 - reserved
                        //             */

                        //             uint32_t capable_messages = (msi_32->message_control >> K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_SHIFT) & K_PCI_MSI_MULTIPLE_MESSAGE_CAPABLE_MASK;
                        //             uint32_t message_count = 1 << capable_messages;
                        //             uint32_t message_data_word = K_PCI_MSI32_MESSAGE_DATA_WORD;
                        //             // uint32_t *messages = k_mem_alloc(sizeof(struct k_pci_msi_message_t) * message_count, sizeof(struct k_pci_msi_message_t));

                        //             /* we also need to tell where the messages are allocated */
                        //             k_pci_write_dword(base_address, dword_offset + K_PCI_MSI_MESSAGE_ADDRESSL_DWORD, messages);

                        //             if(msi_32->message_control & K_PCI_MSI_CAPABILITY_FLAG_64_CAPABLE)
                        //             {
                        //                 /* this function is capable of 64 bit addressing, but we're not, so we'll
                        //                 set the upper 32 bits of the message address to 0. */
                        //                 k_pci_write_dword(base_address, dword_offset + K_PCI_MSI_MESSAGE_ADDRESSH_DWORD, 0);
                        //                 message_data_word = K_PCI_MSI64_MESSAGE_DATA_WORD;
                        //             }

                        //             /* write to the command register the amount of messages we'll be giving this function. Since we're 
                        //             feeling kinda generous, we'll be giving the amount the device asked for. Also, enable msi for this function */
                        //             uint16_t control = msi_32->message_control | (capable_messages << K_PCI_MSI_MULTIPLE_MESSAGE_ENABLE_SHIFT) | K_PCI_MSI_CAPABILITY_FLAG_ENABLE;
                        //             k_pci_write_word(base_address, word_offset + K_PCI_MSI_MESSAGE_CONTROL_WORD, control);
                        //         }
                        //         break;
                        //     }

                        //     dword_offset = capability->next >> 2;
                        //     word_offset = dword_offset << 1;
                        //     capability = (struct k_pci_capability_t *)(header.dwords + dword_offset);
                        // }
                        // while(capability->next);
                    }
                }

                // new_device->base.next = pci_devices;
                // pci_devices = new_device;
            }
        }
    }

    return (struct k_dev_device_t *)pci_devices;
}