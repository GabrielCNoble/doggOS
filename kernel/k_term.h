#ifndef K_TERM_H
#define K_TERM_H

#include <stdint.h>

enum K_VGA_COLOR
{
    K_VGA_COLOR_BLACK,
    K_VGA_COLOR_BLUE,
    K_VGA_COLOR_GREEN,
    K_VGA_COLOR_CYAN,
    K_VGA_COLOR_RED,
    K_VGA_COLOR_MAGENTA,
    K_VGA_COLOR_BROWN,
    K_VGA_COLOR_LIGHT_GREY,
    K_VGA_COLOR_DARK_GREY,
    K_VGA_COLOR_LIGHT_BLUE,
    K_VGA_COLOR_LIGHT_GREEN,
    K_VGA_COLOR_LIGHT_CYAN,
    K_VGA_COLOR_LIGHT_RED,
    K_VGA_COLOR_LIGHT_MAGENTA,
    K_VGA_COLOR_LIGHT_BROWN,
    K_VGA_COLOR_WHITE
};

void k_term_init();

void k_term_check_buffer();

void k_putchar(unsigned char c);

void k_puts(unsigned char *s);

void k_printf(unsigned char *fmt, ...);

uint8_t k_vga_attrib(uint8_t foreground, uint8_t background);

uint16_t k_vga_char(unsigned char c, uint8_t color);

void k_term_clear();

#endif

