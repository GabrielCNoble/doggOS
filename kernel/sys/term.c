#include "term.h"
#include "../rt/alloc.h"
#include "../gfx/k_vga.h"
// #include "k_term.h"
// #include "kb.h"
#include <stdarg.h>
#include "../k_string.h"
#include "../proc/proc.h"
#include "../proc/thread.h"
#include "../defs.h"

extern uint32_t k_gfx_vga_width;
extern uint32_t k_gfx_vga_height;
extern uint16_t *k_gfx_vga_cur_mem_map;

uint16_t *k_sys_term_output_buffer;
uint8_t *k_sys_term_input_buffer;
int32_t k_sys_term_width;
int32_t k_sys_term_height;
uint8_t k_sys_term_color;
int32_t k_sys_term_cursor_x;
int32_t k_sys_term_buffer_cursor_y;
int32_t k_sys_term_display_cursor_y;
int32_t k_sys_term_scroll_y;
// int32_t k_sys_term_display_start_y;
// int32_t display_buffer_start;
// int32_t display_start_x;
// int32_t line_len;


// uint32_t echo_on = 1;

uint16_t k_sys_TerminalChar(unsigned char ch, uint8_t color)
{
    return ((uint16_t)color << 8) | ((uint16_t)ch);
}

void k_sys_TerminalInit()
{
    k_sys_term_width = k_gfx_vga_width;
    k_sys_term_height = K_SYS_TERM_OUTPUT_BUFFER_LINES;

    k_sys_term_output_buffer = k_rt_Malloc(k_sys_term_width * k_sys_term_height * sizeof(uint16_t), 4);
    k_sys_term_input_buffer = k_rt_Malloc(K_SYS_TERM_INPUT_BUFFER_SIZE, 4);
    k_sys_term_cursor_x = 0;
    k_sys_term_display_cursor_y = 0;
    k_sys_term_buffer_cursor_y = 0;
    k_sys_term_scroll_y = 0;
    
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_WHITE, K_SYS_TERM_COLOR_BLACK);

    k_gfx_SetVgaCursorPos(k_sys_term_cursor_x, k_sys_term_display_cursor_y);

    for(int32_t y = 0; y < k_sys_term_height; y++)
    {
        for(int32_t x = 0; x < k_sys_term_width; x++)
        {
            k_sys_term_output_buffer[x + y * k_sys_term_width] = k_sys_TerminalChar(' ', k_sys_term_color);
        }
    }

    // if(k_sys_term_buffer == NULL)
    // {
    //     k_sys_TerminalPrintf("SHIIIT!\n");
    // }
}

void k_sys_TerminalCarriageReturn()
{
    k_sys_term_cursor_x = 0;
}

void k_sys_TerminalPrevLine()
{
    k_sys_term_cursor_x = k_sys_term_width - 1;
    k_sys_term_buffer_cursor_y--;
    if(k_sys_term_buffer_cursor_y < 0)
    {
        k_sys_term_buffer_cursor_y += k_sys_term_height;
    }
    k_sys_term_display_cursor_y--;
}

void k_sys_TerminalNewLine()
{
    // uint8_t color = k_sys_term_color & 0x0f;
    // color |= color << 4;
    // uint16_t newline_char = k_sys_TerminalChar(' ', k_sys_term_color);
    // k_sys_term_output_buffer[k_sys_term_cursor_x + k_sys_term_buffer_cursor_y * k_sys_term_width] = newline_char;
    // shell_buffer[cursor_x + buffer_cursor_y * buffer_width] = k_vga_char(' ', text_color);
    k_sys_term_cursor_x = 0;
    // line_len = 0;
    k_sys_term_buffer_cursor_y = (k_sys_term_buffer_cursor_y + 1) % k_sys_term_height;
    k_sys_term_display_cursor_y++;
}

void k_sys_TerminalBackSpace()
{
    // if(line_len)
    {
        // line_len--;

        if(k_sys_term_cursor_x)
        {
            k_sys_term_cursor_x--;
        }
        else
        {
            k_sys_TerminalPrevLine();

            for(int32_t cursor = k_sys_term_cursor_x; cursor >= 0; cursor--)
            {
                if((k_sys_term_output_buffer[cursor + k_sys_term_buffer_cursor_y * k_sys_term_width] & 0xff) == '\n')
                {
                    k_sys_term_cursor_x = cursor;

                    if(k_sys_term_cursor_x)
                    {
                        k_sys_term_cursor_x--;
                    }

                    break;
                }
            }
        }

        k_sys_term_output_buffer[k_sys_term_cursor_x + k_sys_term_buffer_cursor_y * k_sys_term_width] = k_sys_TerminalChar(' ', k_sys_term_color);
    }
}

void k_sys_TerminalPutSingleChar(unsigned char c)
{    
    if(c == '\n')
    {
        k_sys_TerminalNewLine();
    }
    else if(c == 0x10)
    {
        k_sys_TerminalBackSpace();
    }
    else if(c == '\r')
    {
        k_sys_TerminalCarriageReturn();
    }
    else
    {
        k_sys_term_output_buffer[k_sys_term_cursor_x + k_sys_term_buffer_cursor_y * k_sys_term_width] = k_sys_TerminalChar(c, k_sys_term_color); 
        k_sys_term_cursor_x++;

        if(k_sys_term_cursor_x >= k_sys_term_width)
        {
            k_sys_term_cursor_x = 0;
            k_sys_term_buffer_cursor_y = (k_sys_term_buffer_cursor_y + 1) % k_sys_term_height;
            k_sys_term_display_cursor_y++;
        }
    }
}

void k_sys_TerminalPutChar(char ch)
{
    char str[2] = {};
    str[0] = ch;
    k_sys_TerminalPuts(str);
}

void k_sys_TerminalPuts(char *str)
{
    uint32_t index = 0;
    while(str[index])
    {
        k_sys_TerminalPutSingleChar(str[index]);
        index++;
    }

    int32_t scroll = 0;

    if(k_sys_term_display_cursor_y < 0)
    {
        scroll = k_sys_term_display_cursor_y;
    }
    else if(k_sys_term_display_cursor_y >= (int32_t)k_gfx_vga_height)
    {
        scroll = 1 + (k_sys_term_display_cursor_y - (int32_t)k_gfx_vga_height);
    }

    
    if(scroll > 0)
    {
        uint16_t clear_char = k_sys_TerminalChar(' ', k_sys_term_color);
        int32_t clear_start = (k_sys_term_buffer_cursor_y + 1) % k_sys_term_height;
        int32_t clear_count = scroll;

        for(int32_t clear_index = 0; clear_index < clear_count; clear_index++)
        {
            for(int32_t x = 0; x < k_sys_term_width; x++)
            {
                k_sys_term_output_buffer[x + clear_start * k_sys_term_width] = clear_char;
            }

            clear_start++;
        }
    }

    k_sys_TerminalScroll(scroll);
}

void k_sys_TerminalPrintf(const char *fmt, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, fmt);
    k_vasfmt(buffer, 511, fmt, args);
    va_end(args);

    k_sys_TerminalPuts(buffer);
}

void k_sys_TerminalUpdate()
{
    uint32_t buffer_rows_count = k_sys_term_height - k_sys_term_scroll_y;

    if(buffer_rows_count < k_gfx_vga_height)
    {    
        k_gfx_BlitTextBuffer(k_sys_term_output_buffer + k_sys_term_scroll_y * k_sys_term_width, 0, 0, buffer_rows_count * k_sys_term_width);
        uint32_t wrap_buffer_rows_count = k_gfx_vga_height - buffer_rows_count;
        k_gfx_BlitTextBuffer(k_sys_term_output_buffer, 0, buffer_rows_count, wrap_buffer_rows_count * k_sys_term_width); 
    }
    else
    {
        k_gfx_BlitTextBuffer(k_sys_term_output_buffer + k_sys_term_scroll_y * k_sys_term_width, 0, 0, k_gfx_vga_height * k_sys_term_width);
    }

    k_sys_TerminalSetCursorPos(k_sys_term_cursor_x, k_sys_term_display_cursor_y);
}

void k_sys_TerminalSetColor(uint8_t text_color, uint8_t back_color)
{
    k_sys_term_color = (back_color << 4) | (text_color & 0x0f);
}

void k_sys_TerminalSetCursorPos(int32_t x, int32_t y)
{
    k_sys_term_cursor_x = x;
    k_sys_term_display_cursor_y = y;
    k_gfx_SetVgaCursorPos(k_sys_term_cursor_x, k_sys_term_display_cursor_y);
}

void k_sys_TerminalScroll(int32_t lines)
{
    if(lines)
    {
        k_sys_term_scroll_y += lines;
        k_sys_term_display_cursor_y -= lines;

        if(k_sys_term_scroll_y < 0)
        {
            k_sys_term_scroll_y += k_sys_term_height;
        }
        else if(k_sys_term_scroll_y >= k_sys_term_height)
        {
            k_sys_term_scroll_y %= k_sys_term_height;
        }
    }

    k_sys_TerminalUpdate();
}

void k_sys_TerminalClear()
{
    uint16_t clear_char = k_sys_TerminalChar(' ', k_sys_term_color);

    for(int32_t y = 0; y < k_sys_term_height; y++)
    {
        for(int32_t x = 0; x < k_sys_term_width; x++)
        {
            k_sys_term_output_buffer[x + y * k_sys_term_width] = clear_char;
        }
    }

    k_sys_term_buffer_cursor_y = 0;
    k_sys_term_scroll_y = 0;

    k_sys_TerminalSetCursorPos(0, 0);
    k_sys_TerminalUpdate();
}

// uint32_t k_sys_TerminalReadStream(struct k_io_stream_t *stream, void *data, uint32_t offset, uint32_t size)
// {
//     if(stream)
//     {

//     }
// }

// uint32_t 
// struct k_io_stream_t *k_sys_TerminalOpenStream()
// {
//     struct k_io_stream_t *stream = k_io_AllocStream();

//     stream->read = k_sys_TerminalReadStream;
// }

uint32_t k_sys_TerminalReadLine(char *output, uint32_t buffer_size)
{
    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    uint32_t status;
    uint32_t data_cursor = 0;
    char ch;

    if(!output || !buffer_size)
    {
        return 0;
    }

    do
    {
        while(k_io_ReadStream(current_process->terminal, &ch, 1) == K_STATUS_EMPTY_STREAM)
        {
            k_proc_WaitStream(current_process->terminal);
        }

        // switch(ch)
        // {
        //     case 0x10:
        //         if(data_cursor)
        //         {
        //             data_cursor--;
        //             output[data_cursor] = '\0';    
        //         }
        //     break;

        //     case '\n':
        //     {
        //         output[data_cursor] = '\n';
        //     }
        //     break;
        // }

        if(ch == 0x10)
        {
            if(data_cursor)
            {
                k_sys_TerminalPutChar(ch);
                data_cursor--;
                output[data_cursor] = '\0';
            }
        }
        else
        {
            k_sys_TerminalPutChar(ch);
            if(ch == '\n')
            {
                break;
            }
            output[data_cursor] = ch;
            data_cursor++;
        }
    }
    while(data_cursor < buffer_size - 1);

    output[data_cursor] = '\0';

    return 0;
}

// void k_PrintShell()
// {
//     k_Puts("[Shell]:");
// }

// uintptr_t k_ShellMain(void *data)
// {
//     buffer_width = k_gfx_vga_width;
//     buffer_height = K_SYS_TERM_BUFFER_LINES;

//     text_color = k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_BLACK);
//     unsigned char keyboard_buffer[K_KEYBOARD_MAX_CHARS];
//     shell_buffer = k_rt_Malloc(buffer_width * buffer_height * sizeof(uint16_t), 4);
//     display_cursor_y = 0;
//     cursor_x = 0;
//     buffer_cursor_y = 0;
//     // display_start_x = 0;
//     line_len = 0;
//     display_start_y = 0;

//     k_gfx_SetVgaCursorPos(cursor_x, display_cursor_y);

//     for(int32_t y = 0; y < buffer_height; y++)
//     {
//         for(int32_t x = 0; x < buffer_width; x++)
//         {
//             shell_buffer[x + y * buffer_width] = k_vga_char(' ', text_color);
//         }
//     }

//     k_PrintShell();

//     while(1)
//     {
//         uint32_t char_count = k_kb_ReadKeyboard(keyboard_buffer, K_KEYBOARD_MAX_CHARS);

//         if(char_count)
//         {
//             k_Puts(keyboard_buffer);
//         }

//         if(display_cursor_y < 0)
//         {
//             display_start_y += display_cursor_y;

//             if(display_start_y < 0)
//             {
//                 display_start_y += buffer_height;
//             }

//             display_cursor_y = 0;
//         }
//         else if(display_cursor_y >= (int32_t)k_gfx_vga_height)
//         {
//             int32_t adjust = 1 + (display_cursor_y - (int32_t)k_gfx_vga_height);
//             display_start_y = (display_start_y + adjust) % buffer_height;
//             display_cursor_y = k_gfx_vga_height - 1;
//         }

//         uint32_t buffer_rows_count = buffer_height - display_start_y;

//         if(buffer_rows_count < k_gfx_vga_height)
//         {    
//             k_gfx_BlitTextBuffer(shell_buffer + display_start_y * buffer_width, 0, 0, buffer_rows_count * buffer_width);
//             uint32_t wrap_buffer_rows_count = k_gfx_vga_height - buffer_rows_count;
//             k_gfx_BlitTextBuffer(shell_buffer, 0, buffer_rows_count, wrap_buffer_rows_count * buffer_width); 
//         }
//         else
//         {
//             k_gfx_BlitTextBuffer(shell_buffer + display_start_y * buffer_width, 0, 0, k_gfx_vga_height * buffer_width);
//         }

//         // if(char_count)
//         {
//             k_gfx_SetVgaCursorPos(cursor_x, display_cursor_y);
//         }
//     }
// }