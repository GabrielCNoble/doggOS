#ifndef K_DEV_PS2_H
#define K_DEV_PS2_H

#include "dev.h"
#include "kb.h"
#include "mouse.h"

#define K_DEV_PS2_DATA_PORT             0x60
#define K_DEV_PS2_STATUS_CMD_PORT       0x64
#define K_DEV_PS2_KEYBOARD_IRQ_VECTOR 1
#define K_DEV_PS2_AUX_IRQ_VECTOR 12

enum K_DEV_PS2_STATUS
{
    K_DEV_PS2_STATUS_OUT_BUFFER_FULL    = 1,
    K_DEV_PS2_STATUS_IN_BUFFER_FULL     = 1 << 1,
};

enum K_DEV_PS2_CMDS
{
    K_DEV_PS2_CMD_READ_CMD_BYTE         = 0x20,
    K_DEV_PS2_CMD_WRITE_CMD_BYTE        = 0x60,
    // K_DEV_PS2_CMD_SELF_TEST             = 0xaa,
    K_DEV_PS2_CMD_INTERFACE_TEST        = 0xab,
    K_DEV_PS2_CMD_DIAGNOSTIC_DUMP       = 0xac,
    K_DEV_PS2_CMD_DISABLE_AUX           = 0xa7,
    K_DEV_PS2_CMD_ENABLE_AUX            = 0xa8,
    K_DEV_PS2_CMD_DISABLE_KEYBOARD      = 0xad,
    K_DEV_PS2_CMD_ENABLE_KEYBOARD       = 0xae,
    K_DEV_PS2_CMD_READ_INPUT_PORT       = 0xc0,
    K_DEV_PS2_CMD_READ_OUTPUT_PORT      = 0xd0,
    K_DEV_PS2_CMD_WRITE_OUTPUT_PORT     = 0xd1,
    K_DEV_PS2_CMD_ECHO                  = 0xee,
    K_DEV_PS2_CMD_SET_SCANSET           = 0xf0,
};

/* forward declaration */
struct k_dev_ps2_device_t;

struct k_dev_ps2_funcs_t
{
    uint8_t         (*ReadData)(struct k_dev_ps2_device_t *device);
    void            (*WriteData)(struct k_dev_ps2_device_t *device, uint8_t value);
    uint8_t         (*ReadCmd)(struct k_dev_ps2_device_t *device);
    void            (*WriteCmd)(struct k_dev_ps2_device_t *device, uint8_t value);
};

enum K_DEV_PS2_KEYBOARD_SCANSETS
{
    K_DEV_PS2_KEYBOARD_SCANSET_1 = 1,
    K_DEV_PS2_KEYBOARD_SCANSET_2,
    K_DEV_PS2_KEYBOARD_SCANSET_3,
};

struct k_dev_ps2_keyboard_t
{
    struct k_dev_keyboard_t     base_keyboard;
    struct k_dev_ps2_funcs_t    funcs;
    
    uint16_t                    scan_set;
    uint8_t                     cur_cmd;
    uint8_t                     cur_result;
};

struct k_dev_ps2_mouse_t
{
    struct k_dev_mouse_t        base_mouse;
    struct k_dev_ps2_funcs_t    funcs;
};

struct k_dev_ps2_device_t
{
    union
    {
        struct k_dev_ps2_keyboard_t keyboard;
        struct k_dev_ps2_mouse_t    mouse;
    };
};

struct k_dev_ps2_key_t
{
    uint8_t key;
};

void k_PS2_SetScanset(struct k_dev_ps2_keyboard_t *keyboard, uint8_t scan_set);

uint8_t k_PS2_SendCmd(struct k_dev_ps2_keyboard_t *keyboard, uint8_t cmd);

uint32_t k_PS2_Scanset1Key(struct k_dev_ps2_keyboard_t *keyboard, struct k_dev_keyboard_event_t *event);

void k_PS2_KeyboardInterruptHandler(struct k_dev_ps2_keyboard_t *keyboard);

void k_PS2_MouseInterruptHandler(struct k_dev_ps2_mouse_t *mouse);

#endif