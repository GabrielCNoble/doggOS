#ifndef K_82C59_H
#define K_82C59_H

#include <stdint.h>

// #define K_82C59_CTRL1_ICW1_PORT 0x20
// #define K_82C59_CTRL1_ICW2_PORT 0x21
// #define K_82C59_CTRL1_ICW3_PORT 0x21
// #define K_82C59_CTRL1_ICW4_PORT 0x21
// 
// #define K_82C59_CTRL2_ICW1_PORT 0xa0
// #define K_82C59_CTRL2_ICW2_PORT 0xa1
// #define K_82C59_CTRL2_ICW3_PORT 0xa1
// #define K_82C59_CTRL2_ICW4_PORT 0xa1
// 
// #define K_82C59_CTRL1_OCW1_PORT 0x21
// #define K_82C59_CTRL1_OCW2_PORT 0x20
// #define K_82C59_CTRL1_OCW3_PORT 0x20
// 
// #define K_82C59_CTRL2_OCW1_PORT 0xa1
// #define K_82C59_CTRL2_OCW2_PORT 0xa0
// #define K_82C59_CTRL2_OCW3_PORT 0xa0

/*
    ICW1:
      7-5: A7-A5 of interrupt vector address (MCS-80/85 mode only)
      4:   initiates configuration mode (forced to 1 by the macro)
      3:   1 = level trigger mode, 0 = edge trigger mode
      2:   call address interval. 1 = interval of 4, 0 = interval of 8
      1:   1 = single mode, 0 = cascade mode
      0:   1 = ICW4 needed, 0 = no ICW4 needed
*/

enum K_82C59_ICW1_FIELDS
{
    /* tells the controller to expect 4 initialization words */
    K_82C59_ICW1_FIELD_ICW4_NEEDED        = 1,
    /* selects whether the controller should work in cascade mode  */
    K_82C59_ICW1_FIELD_SINGLE_MODE        = 1 << 1,
    K_82C59_ICW1_FIELD_CALL_ADDR_INTERVAL = 1 << 2,
    K_82C59_ICW1_FIELD_LEVEL_TRIGGER_MODE = 1 << 3,
    /* tells the controller to enter initialization mode */
    K_82C59_ICW1_FIELD_INIT_SELECT        = 1 << 4,
};

#define K_82C59_ICW1(icw4_needed, single_mode, level_trigger) ((uint8_t)(((icw4_needed) * K_82C59_ICW1_FIELD_ICW4_NEEDED) | \
                                                              ((single_mode) * K_82C59_ICW1_FIELD_SINGLE_MODE) | \
                                                              ((level_trigger) * K_82C59_ICW1_FIELD_LEVEL_TRIGGER_MODE) | \
                                                              (K_82C59_ICW1_FIELD_INIT_SELECT)))
                                                    
/*
    ICW2:
      7-0:  A15-A8 of interrupt vector address (MCS-80/85 mode)
      7-3:  bits 7-3 of interrupt vector byte (8086 mode)
*/          
#define K_82C59_ICW2(int_base) ((uint8_t)(int_base))

/*
    ICW3 (master):
      7-0:  1 = slave controller in interrupt line, 0 = no slave in the line
*/
#define K_82C59_M_ICW3(s0, s1, s2, s3, s4, s5, s6, s7) ((uint8_t)((s0) | ((s1) << 1) | ((s2) << 2) | \
                                                                 ((s3) << 3) | ((s4) << 4) | ((s5) << 5) | \
                                                                 ((s6) << 6) | ((s7) << 7)))
/*
    ICW3 (slave):
      2-0:  slave id
*/
#define K_82C59_S_ICW3(sid) ((uint8_t)(sid))

/*
    ICW4:
      4:  1 = special fully nested mode, 0 = not special fully nested mode
      3:  1 = buffered mode, 0 = non-buffered mode
      2:  1 = buffered mode master, 0 = buffered mode slave - (if in buffered mode)
      1:  1 = auto end-of-interrupt, 0 = normal end-of-interrupt
      0:  1 = 8086 mode, 0 = MCS-80/85 mode
*/
enum K_82C59_ICW4_FIELDS
{
    K_82C59_ICW4_FIELD_8086_MODE = 1,
    K_82C59_ICW4_FIELD_AUTO_EOI = 1 << 1,
    K_82C59_ICW4_FIELD_BUF_MASTER = 1 << 2,
    K_82C59_ICW4_FIELD_BUF_MODE = 1 << 3,
    K_82C59_ICW4_FIELD_NEST_MODE = 1 << 4,
};

#define K_82C59_ICW4(cpu_mode, auto_eoi, buff_mode, buff_master, nest_mode) ((uint8_t)(((cpu_mode) * K_82C59_ICW4_FIELD_8086_MODE) | \
                                                                            ((auto_eoi) * K_82C59_ICW4_FIELD_AUTO_EOI) | \
                                                                            ((buff_master) * K_82C59_ICW4_FIELD_BUF_MASTER) | \
                                                                            ((buff_mode) * K_82C59_ICW4_FIELD_BUF_MODE) | \
                                                                            ((nest_mode) * K_82C59_ICW4_FIELD_NEST_MODE)))

// void k_82C59_Init(uint32_t icdw1, uint32_t icdw2);


#endif