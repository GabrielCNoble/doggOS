#ifndef K_82C59_H
#define K_82C59_H

#include <stdint.h>

#define K_82C59_CTRL1_ICW1_PORT 0x20
#define K_82C59_CTRL1_ICW2_PORT 0x21
#define K_82C59_CTRL1_ICW3_PORT 0x21
#define K_82C59_CTRL1_ICW4_PORT 0x21

#define K_82C59_CTRL2_ICW1_PORT 0xa0
#define K_82C59_CTRL2_ICW2_PORT 0xa1
#define K_82C59_CTRL2_ICW3_PORT 0xa1
#define K_82C59_CTRL2_ICW4_PORT 0xa1

#define K_82C59_CTRL1_OCW1_PORT 0x21
#define K_82C59_CTRL1_OCW2_PORT 0x20
#define K_82C59_CTRL1_OCW3_PORT 0x20

#define K_82C59_CTRL2_OCW1_PORT 0xa1
#define K_82C59_CTRL2_OCW2_PORT 0xa0
#define K_82C59_CTRL2_OCW3_PORT 0xa0

void k_82C59_Init(uint32_t icdw1, uint32_t icdw2);


#endif