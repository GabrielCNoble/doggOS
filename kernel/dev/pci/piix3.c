#include "piix3.h"
#include "../../cpu/k_cpu.h"
#include "../../defs.h"
#include "../drv/82C59.h"

uint32_t k_PIIX3_Init(struct k_pci_init_info_t *init_info)
{
    uint32_t base_address;
    
    /* function 0, PCI-to-ISA bridge */
    // base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus, device, 0, 0);
    // k_cpu_OutD(base_address + 0x4c, K_PCI_CONFIG_ADDRESS);
    // uint32_t value = k_cpu_InD(K_PCI_CONFIG_DATA);
    /* set mouse interrupt */
    // value |= (1 << 4) << 16;
    // k_cpu_OutD(value, K_PCI_CONFIG_DATA);


    // k_cpu_OutB(0x11, 0x20);
    // k_cpu_OutB(0x11, 0xa0);

    // k_cpu_OutB(0x20, 0x21);
    // k_cpu_OutB(0x28, 0xa1);   

    // k_cpu_OutB(0x04, 0x21);
    // k_cpu_OutB(0x02, 0xa1);   

    // k_cpu_OutB(0x03, 0x21);
    // k_cpu_OutB(0x03, 0xa1);   
}

uint32_t k_PIIX3_F0_Init(struct k_pci_init_info_t *init_info)
{
    uint32_t icdw1;
    uint32_t icdw2;

    /* 
        ICW1:
            7:5 (icw/ocw select)        - should be 0
            4 (icw/ocw select)          - should be 1, to select init control word write
            3 (edge/level bank select)  - disabled, set to 0
            2 (ADI)                     - ignored, set to 0
            1 (ICW4 write required)     - must be 1
    */
    icdw1 = 0x11;
    icdw2 = 0x11;

    /*
        ICW2:
            7:3 (interrupt vector base) - set to 32 for CTRL-1, 40 for CTRL-2
            2:0 (interrupt req level)   - must be 0
    */
    icdw1 |= 0x20 << 8;
    icdw2 |= 0x28 << 8;

    /*
        ICW3, CTRL-1:
            7:3 (reserved)      - must be 0
            2 (cascade mode)    - must be 1
            1:0 (reserved)      - must be 0
    */
    icdw1 |= 0x04 << 16;


    /*
        ICW3, CTRL-2:
            7:3 (reserved)          - must be 0
            2:0 (slave id mode)     - must be 010
    */
    icdw2 |= 0x02 << 16;
    
    /*
        ICW4:
            7:5 (reserved)                      - must be 0
            4 (SFNM)                            - should be 0
            3 (buffered mode)                   - must be 0
            2 (master/slave in buffered mode)   - must be 0
            1 (auto end of interrupt)           - set to 1
            0 (microprocessor mode)             - should be 1, to select x86
    */
    icdw1 |= 0x03 << 24;
    icdw2 |= 0x03 << 24;

    k_82C59_Init(icdw1, icdw2);

    // k_cpu_OutB(0x11, 0x20);
    // k_cpu_OutB(0x11, 0xa0);

    // k_cpu_OutB(0x20, 0x21);
    // k_cpu_OutB(0x28, 0xa1);   

    // k_cpu_OutB(0x04, 0x21);
    // k_cpu_OutB(0x02, 0xa1);   

    // k_cpu_OutB(0x03, 0x21);
    // k_cpu_OutB(0x03, 0xa1);

    return K_STATUS_OK;   
}

uint32_t k_PIIX3_F1_Init(struct k_pci_init_info_t *init_info)
{
    return K_STATUS_OK;
}

uint32_t k_PIIX3_F2_Init(struct k_pci_init_info_t *init_info)
{
    return K_STATUS_OK;
}