#ifndef K_8042_H
#define K_8042_H

#include <stdint.h>
#include "ps2.h"

// #define K_8042_DATA_PORT 0x60
// #define K_8042_STATUS_CMD_PORT 0x64
// #define K_8042_VECTOR 33
// #define K_8042_MAX_CHARS 32

// enum K_8042_STATUS
// {
//     K_8042_STATUS_OUT_BUFFER_FULL = 1,
//     K_8042_STATUS_IN_BUFFER_FULL = 1 << 1,
// };

// enum K_8042_CMDS
// {
//     K_8042_CMD_READ_CMD_BYTE = 0x20,
//     K_8042_CMD_WRITE_CMD_BYTE = 0x60,
//     K_8042_CMD_SELF_TEST = 0xaa,
//     K_8042_CMD_INTERFACE_TEST = 0xab,
//     K_8042_CMD_DIAGNOSTIC_DUMP = 0xac,
//     K_8042_CMD_DISABLE_KEYBOARD = 0xad,
//     K_8042_CMD_ENABLE_KEYBOARD = 0xae,
//     K_8042_CMD_READ_INPUT_PORT = 0xc0,
//     K_8042_CMD_READ_OUTPUT_PORT = 0xd0,
//     K_8042_CMD_WRITE_OUTPUT_PORT = 0xd1,

// };

struct k_8042_funcs_t
{
    uint8_t (*ReadScancode)(struct k_dev_device_t *device);
    uint8_t (*ReadStatus)(struct k_dev_device_t *device); 
};

// char k_8042_ReadChar();

uint8_t k_8042_ReadScancode();

uint8_t k_8042_ReadStatus();

// char k_8042_TranslateScancode(uint8_t *scancode, uint32_t length);

// uint32_t k_8042_HandleInterrupt();

#endif