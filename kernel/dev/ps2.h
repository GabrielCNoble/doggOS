#ifndef K_DEV_PS2_H
#define K_DEV_PS2_H

#include "dev.h"
#include "kb.h"
#include "mouse.h"

#define K_DEV_PS2_DATA_PORT             0x60
#define K_DEV_PS2_STATUS_CMD_PORT       0x64

enum K_DEV_PS2_STATUS
{
    K_DEV_PS2_STATUS_OUT_BUFFER_FULL    = 1,
    K_DEV_PS2_STATUS_IN_BUFFER_FULL     = 1 << 1,
};

enum K_DEV_PS2_CMDS
{
    K_DEV_PS2_CMD_READ_CMD_BYTE         = 0x20,
    K_DEV_PS2_CMD_WRITE_CMD_BYTE        = 0x60,
    K_DEV_PS2_CMD_SELF_TEST             = 0xaa,
    K_DEV_PS2_CMD_INTERFACE_TEST        = 0xab,
    K_DEV_PS2_CMD_DIAGNOSTIC_DUMP       = 0xac,
    K_DEV_PS2_CMD_DISABLE_AUX           = 0xa7,
    K_DEV_PS2_CMD_ENABLE_AUX            = 0xa8,
    K_DEV_PS2_CMD_DISABLE_KEYBOARD      = 0xad,
    K_DEV_PS2_CMD_ENABLE_KEYBOARD       = 0xae,
    K_DEV_PS2_CMD_READ_INPUT_PORT       = 0xc0,
    K_DEV_PS2_CMD_READ_OUTPUT_PORT      = 0xd0,
    K_DEV_PS2_CMD_WRITE_OUTPUT_PORT     = 0xd1,
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

struct k_dev_ps2_keyboard_t
{
    struct k_dev_keyboard_t base_keyboard;
};

struct k_dev_ps2_mouse_t
{
    struct k_dev_mouse_t base_mouse;
};

struct k_dev_ps2_device_t
{
    union
    {
        struct k_dev_ps2_keyboard_t keyboard;
        struct k_dev_ps2_mouse_t    mouse;
    };
};

void k_PS2_SetScanset(uint8_t scan_set);

void k_PS2_KeyboardInterruptHandler(struct k_dev_ps2_device_t *keyboard);

void k_PS2_MouseInterruptHandler(struct k_dev_ps2_device_t *mouse);

#endif