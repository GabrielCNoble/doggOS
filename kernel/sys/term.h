#ifndef K_SYS_TERM_H
#define K_SYS_TERM_H

#include <stdint.h>

#define K_SYS_TERM_OUTPUT_BUFFER_LINES 256
#define K_SYS_TERM_INPUT_BUFFER_SIZE 4096

enum K_SYS_TERM_COLORS
{
    K_SYS_TERM_COLOR_BLACK,
    K_SYS_TERM_COLOR_BLUE,
    K_SYS_TERM_COLOR_GREEN,
    K_SYS_TERM_COLOR_CYAN,
    K_SYS_TERM_COLOR_RED,
    K_SYS_TERM_COLOR_MAGENTA,
    K_SYS_TERM_COLOR_BROWN,
    K_SYS_TERM_COLOR_LIGHT_GREY,
    K_SYS_TERM_COLOR_DARK_GREY,
    K_SYS_TERM_COLOR_LIGHT_BLUE,
    K_SYS_TERM_COLOR_LIGHT_GREEN,
    K_SYS_TERM_COLOR_LIGHT_CYAN,
    K_SYS_TERM_COLOR_LIGHT_RED,
    K_SYS_TERM_COLOR_LIGHT_MAGENTA,
    K_SYS_TERM_COLOR_LIGHT_BROWN,
    K_SYS_TERM_COLOR_WHITE
};

void k_sys_TerminalInit();

void k_sys_TerminalPutChar(unsigned char c);

void k_sys_TerminalPuts(char *str);

void k_sys_TerminalPrintf(const char *fmt, ...);

void k_sys_TerminalUpdate();

void k_sys_TerminalSetColor(uint8_t text_color, uint8_t back_color);

void k_sys_TerminalSetCursorPos(int32_t x, int32_t y);

void k_sys_TerminalScroll(int32_t lines);

void k_sys_TerminalClear();

uint32_t k_sys_TerminalReadLine(const char *output, uint32_t buffer_size);

#endif