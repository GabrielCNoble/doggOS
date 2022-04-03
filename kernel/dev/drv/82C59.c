#include "82C59.h"
// #include "../../cpu/k_cpu.h"

// void k_82C59_Init(uint32_t icdw1, uint32_t icdw2)
// {   
//     k_cpu_OutB( icdw1        & 0xff, K_82C59_CTRL1_ICW1_PORT);
//     k_cpu_OutB((icdw1 >> 8)  & 0xff, K_82C59_CTRL1_ICW2_PORT);
//     k_cpu_OutB((icdw1 >> 16) & 0xff, K_82C59_CTRL1_ICW3_PORT);
//     k_cpu_OutB((icdw1 >> 24) & 0xff, K_82C59_CTRL1_ICW4_PORT);
// 
//     k_cpu_OutB( icdw2        & 0xff, K_82C59_CTRL2_ICW1_PORT);
//     k_cpu_OutB((icdw2 >> 8)  & 0xff, K_82C59_CTRL2_ICW2_PORT);
//     k_cpu_OutB((icdw2 >> 16) & 0xff, K_82C59_CTRL2_ICW3_PORT);
//     k_cpu_OutB((icdw2 >> 24) & 0xff, K_82C59_CTRL2_ICW4_PORT);
// }