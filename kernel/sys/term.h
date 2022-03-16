#ifndef K_SYS_TERM_H
#define K_SYS_TERM_H

#include <stdint.h>
#include "../io.h"

#define K_SYS_TERM_OUTPUT_BUFFER_LINES 8192
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

uint16_t k_sys_TerminalChar(unsigned char ch, uint8_t color);

void k_sys_TerminalPutChar(char c);

void k_sys_TerminalPuts(char *str);

void k_sys_TerminalPrintf(const char *fmt, ...);

void k_sys_TerminalUpdate();

void k_sys_TerminalSetColor(uint8_t text_color, uint8_t back_color);

void k_sys_TerminalSetCursorPos(int32_t x, int32_t y);

void k_sys_TerminalScroll(int32_t lines);

void k_sys_TerminalClear();

// struct k_io_stream_t *k_sys_TerminalOpenStream();

uint32_t k_sys_TerminalReadLine(char *output, uint32_t buffer_size);

#endif