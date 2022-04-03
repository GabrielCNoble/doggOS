#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../mem/pmap.h"
#include "../../../mem/mngr.h"
#include "../../../defs.h"
#include "../../drv/82C59.h"
#include "../../drv/82C37.h"
#include "../../drv/ide.h"
#include "ide.h"
#include "82C59.h"


uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index)
{
    k_PIIX3_82C59_Init(0, 0);
    
    /* 82C37 dma controllers */
    // k_cpu_OutB(K_82C37_CMD(1, 0, 0, 0, 0, 0, 0, 0), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(K_82C37_CMD(0, 0, 0, 0, 0, 0, 0, 0), K_PIIX3_82C37_CH47_MOD_REG);
    // k_cpu_OutB(K_82C37_MODE(0, 2, 0, 1, 2), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(K_82C37_MODE(1, 1, 0, 1, 2), K_PIIX3_82C37_CH03_MOD_REG);
    // k_cpu_OutB(0x7f, K_PIIX3_82C37_CH0_LOW_PAGE_REG);
    // k_cpu_OutB(0x7e, K_PIIX3_82C37_CH1_LOW_PAGE_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_MASK_CMD_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_BYTE_CMD_REG);
    // k_cpu_OutB(0x10, K_PIIX3_82C37_CH0_BASE_CUR_COUNT_REG);
    // // k_cpu_OutB(0x00, K_PIIX3_82C37_CH0_BASE_CUR_COUNT_REG);
    // 
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH03_CLEAR_BYTE_CMD_REG);
    // k_cpu_OutB(0x10, K_PIIX3_82C37_CH1_BASE_CUR_COUNT_REG);
    // k_cpu_OutB(0x00, K_PIIX3_82C37_CH1_BASE_CUR_COUNT_REG);
    // k_cpu_OutB(0x04, K_PIIX3_82C37_CH03_REQ_REG);
    
    // for(uint32_t x = 0; x < 0xffffff; x++);
    
    // k_sys_TerminalPrintf("%x\n", *page_b);
    // k_cpu_Halt();
    
    k_PIIX3_IDE_Init(bus_index, device_index);
    
    return K_STATUS_OK;
}

// uint32_t k_PIIX3_F0_Init(struct k_pci_init_info_t *init_info)
// {
//     // uint32_t icdw1;
//     // uint32_t icdw2;
// 
//     /* 
//         ICW1:
//             7:5 (icw/ocw select)        - should be 0
//             4 (icw/ocw select)          - should be 1, to select init control word write
//             3 (edge/level bank select)  - disabled, set to 0
//             2 (ADI)                     - ignored, set to 0
//             0 (ICW4 write required)     - must be 1
//     */
//     // icdw1 = 0x11;
//     // icdw2 = 0x11;
// 
//     /*
//         ICW2:
//             7:3 (interrupt vector base) - set to 32 for CTRL-1, 40 for CTRL-2
//             2:0 (interrupt req level)   - must be 0
//     */
//     // icdw1 |= 0x20 << 8;
//     // icdw2 |= 0x28 << 8;
// 
//     /*
//         ICW3, CTRL-1:
//             7:3 (reserved)      - must be 0
//             2 (cascade mode)    - must be 1
//             1:0 (reserved)      - must be 0
//     */
//     // icdw1 |= 0x04 << 16;
// 
// 
//     /*
//         ICW3, CTRL-2:
//             7:3 (reserved)          - must be 0
//             2:0 (slave id mode)     - must be 010
//     */
//     // icdw2 |= 0x02 << 16;
// 
//     /*
//         ICW4:
//             7:5 (reserved)                      - must be 0
//             4 (SFNM)                            - should be 0
//             3 (buffered mode)                   - must be 0
//             2 (master/slave in buffered mode)   - must be 0
//             1 (auto end of interrupt)           - set to 1
//             0 (microprocessor mode)             - should be 1, to select x86
//     */
//     // icdw1 |= 0x03 << 24;
//     // icdw2 |= 0x03 << 24;
// 
//     // k_82C59_Init(icdw1, icdw2);
// 
//     // k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW1_REG);
//     // k_cpu_OutB(K_82C59_ICW2(32), K_PIIX3_82C59_CTRL1_ICW2_REG);
//     // k_cpu_OutB(K_82C59_M_ICW3(0, 0, 1, 0, 0, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW3_REG);
//     // k_cpu_OutB(K_82C59_ICW4(1, 1, 0, 0, 0), K_PIIX3_82C59_CTRL1_ICW4_REG);
//     // 
//     // k_cpu_OutB(K_82C59_ICW1(1, 0, 0), K_PIIX3_82C59_CTRL1_ICW2_REG);
//     // k_cpu_OutB(K_82C59_ICW2(40), K_PIIX3_82C59_CTRL2_ICW2_REG);
//     // k_cpu_OutB(K_82C59_S_ICW3(2), K_PIIX3_82C59_CTRL2_ICW3_REG);
//     // k_cpu_OutB(K_82C59_ICW4(1, 1, 0, 0, 0), K_PIIX3_82C59_CTRL2_ICW4_REG);
//     // k_PIIX3_Init(init_info);
//     return K_STATUS_OK;   
// }

// uint32_t k_PIIX3_F1_Init(struct k_pci_init_info_t *init_info)
// {
//     return K_STATUS_OK;
// }
// 
// uint32_t k_PIIX3_F2_Init(struct k_pci_init_info_t *init_info)
// {
//     return K_STATUS_OK;
// }

// uint32_t k_PIIX3_Init(struct k_pci_init_info_t *init_info)
// {
// 
// }

// void k_PIIX3_82C37_SetMode(uint8_t channel, uint8_t mode)
// {
// 
// }
// 
// uint8_t k_PIIX3_82C37_GetMode(uint8_t channel)
// {
// 
// }
// 
// void k_PIIX3_82C37_RequestTransfer(uint8_t channel)
// {
// 
// }