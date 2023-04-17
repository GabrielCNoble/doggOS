#ifndef K_DEV_KEYBOARD_H
#define K_DEV_KEYBOARD_H

#include <stdint.h>
#include "dev.h"
#include "../io.h"

#define K_DEV_KEYBOARD_IRQ_VECTOR 1
#define K_DEV_KEYBOARD_MAX_CHARS 32


// enum K_DEV_KEYBOARD_MODIFIERS
// {
//     K_DEV_KEYBOARD_MODIFIER_LSHIFT  = 1,
//     K_DEV_KEYBOARD_MODIFIER_RSHIFT  = 1 << 1,
//     K_DEV_KEYBOARD_MODIFIER_LCTRL   = 1 << 2,
//     K_DEV_KEYBOARD_MODIFIER_RCTRL   = 1 << 3,
//     K_DEV_KEYBOARD_MODIFIER_LALT    = 1 << 4,
//     K_
// };


#define K_DEV_KEYBOARD_FIELDS             \
    uint32_t    type;                     \
    uint32_t    lalt:   1;                \
    uint32_t    ralt:   1;                \
    uint32_t    left_ctrl:  1;            \
    uint32_t    right_ctrl:  1;           \
    uint32_t    left_shift: 1;            \
    uint32_t    right_shift: 1;           \
    uint32_t    scroll_lock:  1;          \
    uint32_t    num_lock:  1;             \
    uint32_t    caps_lock:  1;            \

struct k_dev_keyboard_t
{
    struct k_dev_device_t   base_device;
    uint32_t                type;   
    uint32_t                lalt:           1;
    uint32_t                ralt:           1;
    uint32_t                left_ctrl:      1;
    uint32_t                right_ctrl:     1;
    uint32_t                left_shift:     1;
    uint32_t                right_shift:    1;
    uint32_t                scroll_lock:    1;
    uint32_t                num_lock:       1;
    uint32_t                caps_lock:      1;
};

void k_dev_kb_Init();

void k_dev_kb_KeyboardHandler();

uint32_t k_dev_kb_ReadKeyboard(unsigned char *out_buffer, uint32_t buffer_size);

// struct k_io_stream_t *k_kb_OpenStream();


#endif