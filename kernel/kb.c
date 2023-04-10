#include "kb.h"
#include "rt/mem.h"
#include "rt/atm.h"
#include "int/irq.h"
#include "cpu/k_cpu.h"
#include "dev/8042.h"
#include "proc/proc.h"
// #include "dev/pci/piix3/isa.h"
#include "io.h"

char k_kb_scancode_lut[256] =
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
    [0x34] = '.',
    [0x35] = '/',
};

char k_kb_scancode_shift_lut[256] =
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

    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
};

uint8_t k_kb_lshift_down = 0;
uint8_t k_kb_rshift_down = 0;

// uint32_t k_keyboard_buffer_cursor = 0;
// unsigned char k_keyboard_buffer[K_KEYBOARD_MAX_CHARS];

extern void *k_kb_KeyboardHandler_a;
// extern struct k_int_desc_t k_int_idt[];

void k_kb_Init()
{
    k_int_SetInterruptHandler(K_KEYBOARD_IRQ_VECTOR, &k_kb_KeyboardHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3);
    // k_int_idt[32] = K_INT_DESCRIPTOR(&k_kb_KeyboardHandler_a, K_CPU_SEG_SEL(2, 3, 0), 3, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    // k_PIIX3_82C59_EndOfInterrupt();
}

uint32_t k_kb_ReadKeyboard(unsigned char *out_buffer, uint32_t buffer_size)
{
    // if(k_keyboard_buffer_cursor)
    // {
    //     uint32_t copy_size = k_keyboard_buffer_cursor;

    //     if(copy_size > buffer_size)
    //     {
    //         copy_size = buffer_size - 1;
    //     }

    //     k_rt_CopyBytes(out_buffer, k_keyboard_buffer, copy_size);
    //     k_keyboard_buffer_cursor -= copy_size;
    //     out_buffer[copy_size] = '\0';

    //     return copy_size;
    // }

    // return 0;
}

void k_kb_KeyboardHandler()
{
    char ch;
    // k_sys_TerminalPrintf("fuck\n");
    struct k_proc_process_t *current_process;
    
    do
    {
        // struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
        current_process = k_proc_GetFocusedProcess();
        uint8_t scan_code = k_8042_ReadScancode();
        ch = '\0';

        switch(scan_code)
        {
            case 0x2a:
                k_kb_lshift_down = 1;
            break;

            case 0xaa:
                k_kb_lshift_down = 0;
            break;

            case 0x36:
                k_kb_rshift_down = 1;
            break;

            case 0x59:
                k_kb_rshift_down = 0;
            break;

            default:
                ch = k_kb_scancode_lut[scan_code];

                if(ch)
                {
                    if(k_kb_rshift_down || k_kb_lshift_down)
                    {
                        if(ch >= 'a' && ch <= 'z')
                        {
                            ch &= ~0x20;
                        }
                        else if(ch != ' ')
                        {
                            ch = k_kb_scancode_shift_lut[scan_code];
                        }
                    }

                    if(current_process->terminal)
                    {
                        k_io_WriteStreamData(current_process->terminal, &ch, sizeof(char));
                        k_io_SignalStream(current_process->terminal);
                    }
                }
            break;
        }
    }
    while((k_8042_ReadStatus() & K_8042_STATUS_OUT_BUFFER_FULL) && ch);
    k_PIIX3_ISA_EndOfInterrupt(1);
    // asm volatile ("hlt");
}

struct k_io_stream_t *k_kb_OpenStream()
{

}
