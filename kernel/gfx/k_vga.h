#ifndef K_VGA_H
#define K_VGA_H

#include <stdint.h>
#include "../cpu/k_cpu.h"

#define K_GFX_VGA_GFX_REG_ADDR_PORT 0x03ce
#define K_GFX_VGA_GFX_REG_DATA_PORT 0x03cf

enum K_GFX_VGA_GFX_REGS
{
    K_GFX_VGA_GFX_REG_SR = 0x00,
    K_GFX_VGA_GFX_REG_SR_ENABLE = 0x01,
    K_GFX_VGA_GFX_REG_COL_CMP = 0x02,
    K_GFX_VGA_GFX_REG_DATA_ROT = 0x03,
    K_GFX_VGA_GFX_REG_READ_MAP_SEL = 0x04,
    K_GFX_VGA_GFX_REG_GFX_MODE = 0x05,
    K_GFX_VGA_GFX_REG_MISC = 0x06,
    K_GFX_VGA_GFX_REG_COL_DC = 0x07,
    K_GFX_VGA_GFX_REG_BMASK = 0x08,
    K_GFX_VGA_GFX_REG_LAST
};

enum K_GFX_VGA_GFX_MISC_REG_FLAGS
{
    K_GFX_VGA_GFX_MISC_REG_FLAG_ALPHA_DISABLE = 1,
    K_GFX_VGA_GFX_MISC_REG_FLAG_CHAIN_OE_ENABLE = 1 << 1,
};

#define K_GFX_VGA_MEM_MAP_SEL_SHIFT 0x02
#define K_GFX_VGA_MEM_MAP_SEL_MASK 0x0c
#define K_GFX_VGA_MEM_MAP0 0x00
#define K_GFX_VGA_MEM_MAP1 0x01
#define K_GFX_VGA_MEM_MAP2 0x02
#define K_GFX_VGA_MEM_MAP3 0x03


#define K_GFX_VGA_CRT_REG_ADDR_PORT0 0x03b4
#define K_GFX_VGA_CRT_REG_DATA_PORT0 0x03b5
#define K_GFX_VGA_CRT_REG_ADDR_PORT1 0x03d4
#define K_GFX_VGA_CRT_REG_DATA_PORT1 0x03d5

enum K_GFX_VGA_CRT_REGS
{
    K_GFX_VGA_CRT_REG_HORIZONTAL_TOTAL = 0x00,
    K_GFX_VGA_CRT_REG_HDISPLAY_END = 0x01,
    K_GFX_VGA_CRT_REG_START_HBLANKING = 0x02,
    K_GFX_VGA_CRT_REG_END_HBLANKING = 0x03,
    K_GFX_VGA_CRT_REG_START_HRETRACE = 0x04,
    K_GFX_VGA_CRT_REG_END_HRETRACE = 0x05,
    K_GFX_VGA_CRT_REG_VERTICAL_TOTAL = 0x06,
    K_GFX_VGA_CRT_REG_OVERFLOW = 0x07,
    K_GFX_VGA_CRT_REG_PRESET_ROW_SCAN = 0x08,
    K_GFX_VGA_CRT_REG_MAX_SCAN_LINE = 0x09,
    K_GFX_VGA_CRT_REG_CURSOR_START = 0x0a,
    K_GFX_VGA_CRT_REG_CURSOR_END = 0x0b,
    K_GFX_VGA_CRT_REG_START_ADDRH = 0x0c,
    K_GFX_VGA_CRT_REG_START_ADDRL = 0x0d,
    K_GFX_VGA_CRT_REG_CURSOR_LOCH = 0x0e,
    K_GFX_VGA_CRT_REG_CURSOR_LOCL = 0x0f,
    K_GFX_VGA_CRT_REG_VRETRACE_START = 0x10,
    K_GFX_VGA_CRT_REG_VRETRACE_END = 0x11,
    K_GFX_VGA_CRT_REG_VDISPLAY_END = 0x12,
    K_GFX_VGA_CRT_REG_OFFSET = 0x13,
    K_GFX_VGA_CRT_REG_UNDERLINE_LOC = 0x14,
    K_GFX_VGA_CRT_REG_START_VBLANKING = 0x15,
    K_GFX_VGA_CRT_REG_END_VBLANKING = 0x16,
    K_GFX_VGA_CRT_REG_MODE_CONTROL = 0x17,
    K_GFX_VGA_CRT_REG_LINE_COMPARE = 0x18,
    K_GFX_VGA_CRT_REG_LAST
};

#define K_GFX_VGA_CRT_CURSOR_SCAN_LINE_MASK 0x1f

enum K_GFX_VGA_CRT_CURSOR_START_REG_FLAGS
{
    K_GFX_VGA_CRT_CURSOR_START_REG_FLAG_CURSOR_DISABLE = 1 << 5,
};

#define K_GFX_VGA_SEQ_REG_ADDRESS_PORT 0x03c4
#define K_GFX_VGA_SEQ_REG_DATA_PORT 0x03c5

enum K_GFX_VGA_SEQ_REGS
{
    K_GFX_VGA_SEQ_REG_RESET = 0x0,
    K_GFX_VGA_SEQ_REG_CLOCK_MODE = 0x1,
    K_GFX_VGA_SEQ_REG_MAP_MASK = 0x2,
    K_GFX_VGA_SEQ_REG_CHAR_MAP_SELECT = 0x3,
    K_GFX_VGA_SEQ_REG_MEMORY_MODE = 0x4,
    K_GFX_VGA_SEQ_REG_LAST
};

enum K_GFX_VGA_SEQ_MEMORY_MODE_REG_FLAGS
{
    K_GFX_VGA_SEQ_MEMORY_MODE_REG_FLAG_EXT_MEM = 1 << 1,
    K_GFX_VGA_SEQ_MEMORY_MODE_REG_OE_DISABLE = 1 << 2,
    K_GFX_VGA_SEQ_MEMORY_MODE_REG_CHAIN4_ENABLE = 1 << 3
};

enum K_GFX_VGA_EXT_REGS
{
    K_GFX_VGA_EXT_REG_MISC = 0,
    K_GFX_VGA_EXT_REG_FEAT_CTRL = 1,
    K_GFX_VGA_EXT_REG_FEAT_CTRLM = 2,
    K_GFX_VGA_EXT_REG_FEAT_CTRLC = 3,
    K_GFX_VGA_EXT_REG_INPUT_STATUS0 = 4,
    K_GFX_VGA_EXT_REG_INPUT_STATUS1 = 5,
    K_GFX_VGA_EXT_REG_INPUT_STATUS1M = 6,
    K_GFX_VGA_EXT_REG_INPUT_STATUS1C = 7,
    K_GFX_VGA_EXT_REG_LAST
};

#define K_GFX_VGA_EXT_REG_MISC_READ_PORT 0x03cc
#define K_GFX_VGA_EXT_REG_MISC_WRITE_PORT 0x03c2
#define K_GFX_VGA_EXT_REG_FEAT_CTRLM_READ_PORT 0x03ca
#define K_GFX_VGA_EXT_REG_FEAT_CTRLM_WRITE_PORT 0x03ba
#define K_GFX_VGA_EXT_REG_FEAT_CTRLC_READ_PORT 0x03ca
#define K_GFX_VGA_EXT_REG_FEAT_CTRLC_WRITE_PORT 0x03da
#define K_GFX_VGA_EXT_REG_INPUT_STATUS0_READ_PORT 0x03c2
#define K_GFX_VGA_EXT_REG_INPUT_STATUS1M_READ_PORT 0x03ba
#define K_GFX_VGA_EXT_REG_INPUT_STATUS1C_READ_PORT 0x03da

enum K_GFX_VGA_EXT_MISC_REG_FLAGS
{
    K_GFX_VGA_EXT_MISC_REG_FLAG_IO_ADDR_SEL = 1,
    K_GFX_VGA_EXT_MISC_REG_FLAG_RAM_ENABLE = 1 << 1,
    K_GFX_VGA_EXT_MISC_REG_FLAG_CLOCK_SEL_28M_CLOCK = 1 << 2,
    K_GFX_VGA_EXT_MISC_REG_FLAG_ODD_EVEN_PAGE_SEL = 1 << 5,
    K_GFX_VGA_EXT_MISC_REG_FLAG_HSYNC_POLARITY = 1 << 6,
    K_GFX_VGA_EXT_MISC_REG_FLAG_VSYNC_POLARITY = 1 << 7,
};

enum K_GFX_VGA_EXT_INPUT_STATUS1_REG_FLAGS
{
    K_GFX_VGA_EXT_INPUT_STATUS1_REG_FLAG_DISP_DISABLED = 1,
    K_GFX_VGA_EXT_INPUT_STATUS1_REG_FLAG_VERT_RETRACE = 1 << 3,
};


enum K_GFX_VGA_GFX_MODES
{
    K_GFX_VGA_GFX_MODE_GRAPHIC = 0,
    K_GFX_VGA_GFX_MODE_ALPHANUMERIC
};

void k_gfx_InitVga();

void k_gfx_WriteVgaGfxReg(uint8_t reg, uint8_t value);

uint8_t k_gfx_ReadVgaGfxReg(uint8_t reg);

void k_gfx_WriteVgaCrtReg(uint8_t reg, uint8_t value);

uint8_t k_gfx_ReadVgaCrtReg(uint8_t reg);

void k_gfx_WriteVgaSeqReg(uint8_t reg, uint8_t value);

uint8_t k_gfx_ReadVgaSeqReg(uint8_t reg);

void k_gfx_WriteVgaExtReg(uint8_t reg, uint8_t value);

uint8_t k_gfx_ReadVgaExtReg(uint8_t reg);

/*
=============================================================================================
=============================================================================================
=============================================================================================
*/

void k_gfx_SetVgaMemMap(uint32_t map);

void k_gfx_SetVgaCursorPos(uint32_t x, uint32_t y);

void k_gfx_ToggleVgaCursor(uint32_t enable);

void k_gfx_SetVgaGfxMode(uint32_t mode);

void k_gfx_FillSquare(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);

void k_gfx_BlitTextBuffer(uint16_t *text, uint16_t start_x, uint16_t start_y, uint16_t count);

// uint16_t k_gfx_VgaChar(char ch, char text_color, char back_color);

#endif