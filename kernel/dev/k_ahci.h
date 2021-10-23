#ifndef K_AHCI_H
#define K_AHCI_H

#include <stdint.h>

struct k_ahci_port_regs_t
{
    uint32_t cmd_list_base_ldw;
    uint32_t cmd_list_base_hdw;
    uint32_t fis_base_ldw;
    uint32_t fis_base_hdw;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command_and_status;
    uint32_t reserved0;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notification;
    uint32_t fis_switch_control;
    uint32_t device_sleep;
    uint32_t reserved1;
    uint32_t vendor_specific[4];
};

struct k_ahci_regs_t
{
    uint32_t host_capabilities;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t ports_implemented;
    uint16_t version_minor;
    uint16_t version_major;
    uint32_t ccc_control;
    uint32_t ccc_ports;
    uint32_t encl_mng_location;
    uint32_t encl_mng_control;
    uint32_t host_capabilites_ext;
    uint32_t bios_os_handoff_ctrl_status;

    uint8_t reserved0[52];
    uint8_t reserved1[64];
    uint8_t reserved2[96];

    struct k_ahci_port_regs_t ports[32];
};

struct k_ahci_device_t
{
    struct k_ahci_regs_t *regs;
};

enum K_AHCI_FIS_TYPE
{
    K_AHCI_FIS_TYPE_H2D = 0x27,
    K_AHCI_FIS_TYPE_D2H = 0x34,
    K_AHCI_FIS_TYPE_BIST = 0x58,
};

struct k_ahci_fis_t
{
    uint8_t type;

    union
    {
        struct
        {
            uint8_t pm_c;
            uint8_t command;
            uint8_t featuresl;
            uint8_t lba0[3];
            uint8_t device;
            uint8_t lba1[3];
            uint8_t featuresh;
            uint16_t count;
            uint8_t icc;
            uint8_t control;
            uint32_t reserved;
        } h2d;

        struct
        {
            uint8_t pm_i;
            uint8_t status;
            uint8_t error;
            uint8_t lba0[3];
            uint8_t device;
            uint8_t lba1[3];
            uint8_t reserved0;
            uint16_t count;
            uint16_t reserved1;
            uint32_t reserved2;
        } d2h;

        struct
        {

        } bist;
    };
};

struct k_ahci_recv_fis_t
{
    uint32_t dma_setup_fis[7];
    uint32_t reserved0;
    uint32_t pio_setup_fis[5];
    uint32_t reserved1[3];
    
};

enum K_AHCI_CMD_FLAG
{
    K_AHCI_CMD_FLAG_PREFETCH = 1 << 7,
    K_AHCI_CMD_FLAG_WRITE = 1 << 6,
    K_AHCI_CMD_FLAG_ATAPI = 1 << 5,
};

struct k_ahci_prd_entry_t
{
    uint32_t data_base_ldw;
    uint32_t data_base_hdw;
    uint32_t reserved;
    uint32_t byte_count_i;
};

struct k_ahci_cheader_t
{
    /* 
        clf - command fis length 
        w - means a write to the device
        p - prefetch physical region descriptors
    */
    uint8_t cflawp;

    /*
        r - indicates this command is part of a software reset sequence
        b - indicates this command is sending a BIST fis
    */
    uint8_t rbcmpm;

    /* physical region descriptor table length, in entries of 4 dwords */
    uint16_t prdtl;

    /* how many bytes of the physical region descriptor table have been transfered */
    uint32_t prdbc;
    
    /* command table base address */
    uint32_t ctba_ldw;
    uint32_t ctba_udw;
    uint32_t reserved[4];
};

struct k_ahci_ctable_t
{
    union
    {
        struct k_ahci_fis_t cfis;
        uint64_t cfis_dwords[16];
    };

    uint32_t acmd[4];
    uint32_t reserved[12];
    struct k_ahci_prd_entry_t entries[1];
};

#endif