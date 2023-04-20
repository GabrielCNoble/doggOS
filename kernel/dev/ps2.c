#include "ps2.h"

#define K_PS2_SCANSET1_BREAK_MASK       0x80
#define K_PS2_SCANSET1_LSHIFT_DOWN      0x2a
#define K_PS2_SCANSET1_LSHIFT_UP        0xaa

#define K_PS2_SCANSET1_RSHIFT_DOWN      0x36
#define K_PS2_SCANSET1_RSHIFT_UP        0xb6
#define K_PS2_SCANSET1_ENTER_DOWN       0x1c
#define K_PS2_SCANSET1_ENTER_UP         0x9c


struct k_dev_ps2_key_t k_ps2_scanset1_key_lut1[] = {
    [0x29] = {.key = 1},
    [0x02] = {.key = 2},
    [0x03] = {.key = 3},
    [0x04] = {.key = 4},
    [0x05] = {.key = 5},
    [0x06] = {.key = 6},
    [0x07] = {.key = 7},
    [0x08] = {.key = 8},
    [0x09] = {.key = 9},
    [0x0a] = {.key = 10},
    [0x0b] = {.key = 11},
    [0x0c] = {.key = 12},
    [0x0d] = {.key = 13},
    [0x0e] = {.key = K_DEV_KEYBOARD_BACKSPACE_KEY},
    [0x0f] = {.key = K_DEV_KEYBOARD_TAB_KEY},
    [0x10] = {.key = 17},
    [0x11] = {.key = 18},
    [0x12] = {.key = 19},
    [0x13] = {.key = 20},
    [0x14] = {.key = 21},
    [0x15] = {.key = 22},
    [0x16] = {.key = 23},
    [0x17] = {.key = 24},
    [0x18] = {.key = 25},
    [0x19] = {.key = 26},
    [0x1a] = {.key = 27},
    [0x1b] = {.key = 28},
    [0x2b] = {.key = 29},
    [0x3a] = {.key = K_DEV_KEYBOARD_CASPLOCK_KEY},
    [0x1e] = {.key = 31},
    [0x1f] = {.key = 32},
    [0x20] = {.key = 33},
    [0x21] = {.key = 34},
    [0x22] = {.key = 35},
    [0x23] = {.key = 36},
    [0x24] = {.key = 37},
    [0x25] = {.key = 38},
    [0x26] = {.key = 39},
    [0x27] = {.key = 40},
    [0x28] = {.key = 41},
    [0x1c] = {.key = K_DEV_KEYBOARD_ENTER_KEY},
    [0x2a] = {.key = K_DEV_KEYBOARD_LSHIFT_KEY},
    [0x2c] = {.key = 46},

    [0x2d] = {.key = 47},
    [0x2e] = {.key = 48},
    [0x2f] = {.key = 49},
    [0x30] = {.key = 50},
    [0x31] = {.key = 51},
    [0x32] = {.key = 52},
    [0x33] = {.key = 53},
    [0x34] = {.key = 54},
    [0x35] = {.key = 55},
    [0x36] = {.key = K_DEV_KEYBOARD_RSHIFT_KEY},
    [0x1d] = {.key = K_DEV_KEYBOARD_LCTRL_KEY},
    [0x38] = {.key = 60},
    [0x39] = {.key = 61},

    [0x45] = {.key = K_DEV_KEYBOARD_NUMLOCK_KEY},
    [0x47] = {.key = 91},
    [0x4b] = {.key = 92},
    [0x4f] = {.key = 93},
    [0x48] = {.key = 96},
    [0x4c] = {.key = 97},
    [0x50] = {.key = 98},
    [0x52] = {.key = 99},
    [0x37] = {.key = 100},
    [0x49] = {.key = 101},
    [0x4d] = {.key = 102},
    [0x51] = {.key = 103},
    [0x53] = {.key = 104},
    [0x4a] = {.key = 105},
    [0x4e] = {.key = 106},
    [0x01] = {.key = 110},
    [0x3b] = {.key = 112},
    [0x3c] = {.key = 113},
    [0x3d] = {.key = 114},
    [0x3e] = {.key = 115},
    [0x3f] = {.key = 116},
    [0x40] = {.key = 117},
    [0x41] = {.key = 118},
    [0x42] = {.key = 119},
    [0x43] = {.key = 120},
    [0x44] = {.key = 121},
    [0x57] = {.key = 122},
    [0x58] = {.key = 123},
    [0x46] = {.key = 125},
};

struct k_dev_ps2_key_t k_ps2_scanset1_key_lut2[] = {
    [0x1c] = {.key = K_DEV_KEYBOARD_KP_ENTER_KEY},
    [0x1d] = {.key = K_DEV_KEYBOARD_RCTRL_KEY},
    [0x38] = {.key = K_DEV_KEYBOARD_RALT_KEY},
    [0x47] = {.key = 80},
    [0x48] = {.key = 83},
    [0x49] = {.key = 85},
    [0x4b] = {.key = 79},
    [0x4d] = {.key = 89},
    [0x4f] = {.key = 81},
    [0x50] = {.key = 84},
    [0x51] = {.key = 86},
    [0x52] = {.key = 75},
    [0x53] = {.key = 76},
};

void k_PS2_SetScanset(struct k_dev_ps2_keyboard_t *keyboard, uint8_t scan_set)
{
    
}

uint8_t k_PS2_SendCmd(struct k_dev_ps2_keyboard_t *keyboard, uint8_t cmd)
{
    keyboard->cur_cmd = cmd;
    keyboard->funcs.WriteCmd((struct k_dev_ps2_device_t *)keyboard, cmd);
}

uint32_t k_PS2_Scanset1Key(struct k_dev_ps2_keyboard_t *keyboard, struct k_dev_keyboard_event_t *event)
{
    uint8_t data = keyboard->funcs.ReadData((struct k_dev_ps2_device_t *)keyboard);
    struct k_dev_ps2_key_t *key_lut = k_ps2_scanset1_key_lut1;

    if(data == 0xe0)
    {
        data = keyboard->funcs.ReadData((struct k_dev_ps2_device_t *)keyboard);

        switch(data & (~K_PS2_SCANSET1_BREAK_MASK))
        {
            case K_PS2_SCANSET1_LSHIFT_DOWN:
            case K_PS2_SCANSET1_RSHIFT_DOWN:
                return 0;
        }

        key_lut = k_ps2_scanset1_key_lut2;
    }

    if(data & K_PS2_SCANSET1_BREAK_MASK)
    {
        event->type = K_DEV_KEYBOARD_EVENT_KEY_UP;
    }
    else
    {
        event->type = K_DEV_KEYBOARD_EVENT_KEY_DOWN;
    }

    data &= ~K_PS2_SCANSET1_BREAK_MASK;

    event->key = key_lut[data].key;

    return 1;
}

void k_PS2_KeyboardInterruptHandler(struct k_dev_ps2_keyboard_t *keyboard)
{
    struct k_dev_keyboard_event_t event = {};
    
    // if(keyboard->cur_cmd == 0)
    // {
    while(keyboard->funcs.ReadCmd((struct k_dev_ps2_device_t *)keyboard) & K_DEV_PS2_STATUS_OUT_BUFFER_FULL)
    {
        if(k_PS2_Scanset1Key(keyboard, &event))
        {
            k_dev_kb_KeyboardHandler((struct k_dev_keyboard_t *)keyboard, &event);
        }
    }
    // }

    keyboard->cur_cmd = 0;
}

void k_PS2_MouseInterruptHandler(struct k_dev_ps2_mouse_t *mouse)
{

}