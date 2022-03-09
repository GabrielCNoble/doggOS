#include "kb.h"
#include "rt/mem.h"
#include "int/int.h"
#include "cpu/k_cpu.h"

char k_scancode_lut[256] = 
{
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0a] = '9',
    [0x0b] = '0',
    [0x0c] = '-',
    [0x0d] = '=',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1e] = 'a',
    [0x1f] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x2c] = 'z',
    [0x2d] = 'x',
    [0x2e] = 'c',
    [0x2f] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x39] = ' ',
    [0x1c] = '\n',
    [0x0e] = 0x10,
    [0x33] = ',',
    [0x34] = '.'
};

char k_scancode_shift_lut[256] = 
{
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '¨',
    [0x08] = '&',
    [0x09] = '*',
    [0x0a] = '(',
    [0x0b] = ')',
    [0x0c] = '_',
    [0x0d] = '+',

    [0x27] = ':',
    [0x28] = '\"',

    [0x33] = '>',
    [0x34] = '<'
};

uint8_t k_lshift_down = 0;
uint8_t k_rshift_down = 0;

uint32_t k_keyboard_buffer_cursor = 0;
unsigned char k_keyboard_buffer[K_KEYBOARD_MAX_CHARS];

extern void *k_kb_KeyboardHandler_a;

void k_kb_Init()
{
    k_int_SetInterruptHandler(K_KEYBOARD_VECTOR, &k_kb_KeyboardHandler_a, K_CPU_SEG_SEL(6, 3, 0), 3);
}

uint32_t k_kb_ReadKeyboard(unsigned char *out_buffer, uint32_t buffer_size)
{
    if(k_keyboard_buffer_cursor)
    {
        uint32_t copy_size = k_keyboard_buffer_cursor;

        if(copy_size > buffer_size)
        {
            copy_size = buffer_size - 1;
        }

        k_rt_CopyBytes(out_buffer, k_keyboard_buffer, copy_size);
        k_keyboard_buffer_cursor -= copy_size;
        out_buffer[copy_size] = '\0';

        return copy_size;
    }

    return 0;
}

void k_kb_KeyboardHandler()
{
    uint8_t scan_code = k_cpu_InB(K_KEYBOARD_DATA_PORT);
    uint8_t status;
    unsigned char c;

    do
    {
        switch(scan_code)
        {
            case 0x2a:
                k_lshift_down = 1;
            break;

            case 0xaa:
                k_lshift_down = 0;
            break;

            case 0x36:
                k_rshift_down = 1;
            break;

            case 0x59:
                k_rshift_down = 0;
            break;

            default:
                c = k_scancode_lut[scan_code];

                if(c)
                {
                    if(k_rshift_down || k_lshift_down)
                    {
                        if(c >= 'a' && c <= 'z')
                        {
                            c &= ~0x20;
                        }
                        else if(c != ' ')
                        {
                            c = k_scancode_shift_lut[scan_code];
                        }
                    }

                    k_keyboard_buffer[k_keyboard_buffer_cursor] = c;
                    k_keyboard_buffer_cursor++;
                }
            break;
        }

        status = k_cpu_InB(K_KEYBOARD_STATUS_CMD_PORT);
    }
    while((status & K_KEYBOARD_STATUS_OUT_BUFFER_FULL) && k_keyboard_buffer_cursor < K_KEYBOARD_MAX_CHARS);
}