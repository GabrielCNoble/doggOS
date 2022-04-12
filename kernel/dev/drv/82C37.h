#ifndef K_82C37_H
#define K_82C37_H

#include <stdint.h>

enum K_82C37_CMD_FLAGS
{
    K_82C37_CMD_FLAG_MEM2MEM        = 1,
    K_82C37_CMD_FLAG_CH0_ADDR_HOLD  = 1 << 1,
    K_82C37_CMD_FLAG_CTRL_DISABLE   = 1 << 2,
    K_82C37_CMD_FLAG_COMPRESSED_TIM = 1 << 3,
    K_82C37_CMD_FLAG_ROT_PRIORITY   = 1 << 4,
    K_82C37_CMD_FLAG_EXT_WRITE      = 1 << 5,
    K_82C37_CMD_FLAG_DREQ_SENSE_LOW = 1 << 6,
    K_82C37_CMD_FLAG_DACK_SENSE_LOW = 1 << 7,
};

#define K_82C37_CMD(mem_to_mem, ch0_addr_hold, disable, compressed_tim, rot_priority, write, dreq_sense_low, dack_sense_low) \
        ((uint8_t)((mem_to_mem && 1) | \
                  ((ch0_addr_hold && 1) << 1) | \
                  ((disable && 1) << 2) | \
                  ((compressed_tim && 1) << 3) | \
                  ((rot_priority && 1) << 4) | \
                  ((write && 1) << 5) | \
                  ((dreq_sense_low && 1) << 6) | \
                  ((dack_sense_low) && 1) << 7))

enum K_82C37_MODE_FIELDS
{
    K_82C37_MODE_FIELD_CH0_SEL    = 0,
    K_82C37_MODE_FIELD_CH1_SEL    = 1,
    K_82C37_MODE_FIELD_CH2_SEL    = 2,
    K_82C37_MODE_FIELD_CH3_SEL    = 3,
    K_82C37_MODE_FIELD_VRFY_TX    = 0 << 2,
    K_82C37_MODE_FIELD_W_TX       = 1 << 2,
    K_82C37_MODE_FIELD_R_TX       = 2 << 2,
    K_82C37_MODE_FIELD_ILL_TX     = 3 << 2,
    K_82C37_MODE_FIELD_AUTOINIT   = 1 << 4,
    K_82C37_MODE_FIELD_ADDR_INC   = 1 << 5,
    K_82C37_MODE_FIELD_DEMAND_MOD = 0 << 6,
    K_82C37_MODE_FIELD_SINGLE_MOD = 1 << 6,
    K_82C37_MODE_FIELD_BLOCK_MOD  = 2 << 6,
    K_82C37_MODE_FIELD_CASC_MOD   = 3 << 6,
};

#define K_82C37_MODE(channel, transf_type, auto_init, addr_inc, transf_mode) \
            ((uint8_t) ( (channel) | \
                        ((transf_type) << 2 ) | \
                        ((auto_init) << 4) | \
                        ((addr_inc) << 5) | \
                        ((transf_mode) << 6) ))
        

enum K_82C37_REQ_FIELDS
{
    K_82C37_REQ_FIELD_CH0_SEL = 0,
    K_82C37_REQ_FIELD_CH1_SEL = 1,
    K_82C37_REQ_FIELD_CH2_SEL = 2,
    K_82C37_REQ_FIELD_CH3_SEL = 3,
    K_82C37_REQ_FIELD_REQUEST = 1 << 2,
};

enum K_82C37_STATUS_FIELDS
{
    K_82C37_STATUS_FIELD_CH0_TC   = 1,
    K_82C37_STATUS_FIELD_CH1_TC   = 1 << 1,
    K_82C37_STATUS_FIELD_CH2_TC   = 1 << 2,
    K_82C37_STATUS_FIELD_CH3_TC   = 1 << 3,
    K_82C37_STATUS_FIELD_CH0_REQ  = 1 << 4,
    K_82C37_STATUS_FIELD_CH1_REQ  = 1 << 5,
    K_82C37_STATUS_FIELD_CH2_REQ  = 1 << 6,
    K_82C37_STATUS_FIELD_CH3_REQ  = 1 << 7,
};



// uint32_t k_82C37_SetMode(uint8_t channel, uint8_t fields);
// 
// uint8_t k_82C37_GetMode(uint8_t channel);
// 
// uint32_t k_82C37_RequestTransfer(uint8_t channel, uint8_t transfer_mode);

#endif