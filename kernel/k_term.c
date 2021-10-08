#include "k_term.h"
#include "k_string.h"
#include <stdint.h>
#include <stdarg.h>

uint32_t vga_width;
uint32_t vga_height;
uint32_t vga_cur_row;
uint32_t vga_cur_column;
// uint16_t *vga_buffer;

uint8_t vga_foreground_color;
uint8_t vga_background_color;
uint8_t vga_cur_color;

extern uint16_t *k_gfx_vga_cur_mem_map;

// unsigned char buffer[4096];

void k_term_init()
{
    vga_width = 80;
    vga_height = 25;
    // vga_buffer = (uint16_t *) 0xb8000;
    // vga_buffer = (uint16_t *) 0xa0000;
    // vga_buffer = (uint16_t *)0xf80000;
    // vga_cur_color = k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_BLACK);
    // vga_foreground_color = K_VGA_COLOR_WHITE;
    // vga_background_color = K_VGA_COLOR_BLACK;
    k_term_SetColors(K_VGA_COLOR_WHITE, K_VGA_COLOR_BLACK);
    k_term_clear();

    // for(uint32_t index = 0; index < 4096; index++)
    // {
    //     buffer[index] = 'f';
    // }
}

void k_term_check_buffer()
{
    // for(uint32_t index = 512; index < 4096; index++)
    // {
    //     if(buffer[index] != 'f')
    //     {
    //         k_term_clear();
    //         k_puts("well, the buffer is corrupt!\n");
    //         return;
    //     }
    // }
}

void k_putchar(char c)
{
    if(c == '\n')
    {
        vga_cur_column = 0;
        vga_cur_row++;
    }
    else if(c == '\r')
    {
        vga_cur_column = 0;
    }
    else
    {
        uint16_t entry = k_vga_char(c, vga_cur_color);
        k_gfx_vga_cur_mem_map[vga_cur_column + vga_cur_row * vga_width] = entry;

        vga_cur_column++;
        if(vga_cur_column >= vga_width)
        {
            vga_cur_column = 0;
            vga_cur_row++;
        }
    }

    if(vga_cur_row >= vga_height)
    {
        vga_cur_row = 0;
    }
}

void k_puts(char *s)
{
    uint32_t index = 0;

    while(s[index])
    {
        k_putchar(s[index]);
        index++;
    }
}
// #define SIZE 512

void k_printf(char *fmt, ...)
{
    va_list args;
    char buffer[512];

    va_start(args, fmt);
    k_vasfmt(buffer, 511, fmt, args);
    va_end(args);

    k_puts(buffer);
}

uint8_t k_vga_attrib(uint8_t foreground, uint8_t background)
{
    return foreground | (background << 4);
}

uint16_t k_vga_char(unsigned char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

void k_term_clear()
{
    vga_cur_row = 0;
    vga_cur_column = 0;
    
    uint16_t entry = k_vga_char(' ', vga_cur_color);

    for(uint32_t row = 0; row < vga_height; row++)
    {
        uint32_t offset = row * vga_width;

        for(uint32_t column = 0; column < vga_width; column++)
        {
            k_gfx_vga_cur_mem_map[column + offset] = entry;
        }
    }
}

void k_term_SetColors(uint8_t foreground, uint8_t background)
{
    vga_cur_color = k_vga_attrib(foreground, background);
}

void k_term_SetCursorRow(uint32_t row)
{
    vga_cur_row = row;
    vga_cur_column = 0;
}