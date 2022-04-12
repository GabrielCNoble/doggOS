#ifndef K_KB_H
#define K_KB_H

#include <stdint.h>
#include "io.h"

// #define K_KEYBOARD_DATA_PORT 0x60
// #define K_KEYBOARD_STATUS_CMD_PORT 0x64
#define K_KEYBOARD_IRQ_VECTOR 33
#define K_KEYBOARD_MAX_CHARS 32

// enum K_KEYBOARD_STATUS
// {
//     K_KEYBOARD_STATUS_OUT_BUFFER_FULL = 1,
//     K_KEYBOARD_STATUS_IN_BUFFER_FULL = 1 << 1,
// };

// enum K_KEYBOARD_CMDS
// {
//     K_KEYBOARD_CMD_READ_CMD_BYTE = 0x20,
//     K_KEYBOARD_CMD_WRITE_CMD_BYTE = 0x60,
//     K_KEYBOARD_CMD_SELF_TEST = 0xaa,
//     K_KEYBOARD_CMD_INTERFACE_TEST = 0xab,
//     K_KEYBOARD_CMD_DIAGNOSTIC_DUMP = 0xac,
//     K_KEYBOARD_CMD_DISABLE_KEYBOARD = 0xad,
//     K_KEYBOARD_CMD_ENABLE_KEYBOARD = 0xae,
//     K_KEYBOARD_CMD_READ_INPUT_PORT = 0xc0,
//     K_KEYBOARD_CMD_READ_OUTPUT_PORT = 0xd0,
//     K_KEYBOARD_CMD_WRITE_OUTPUT_PORT = 0xd1,

// };

void k_kb_Init();

void k_kb_KeyboardHandler();

uint32_t k_kb_ReadKeyboard(unsigned char *out_buffer, uint32_t buffer_size);

// struct k_io_stream_t *k_kb_OpenStream();


#endif