#include "k_gfx_vga.h"
#include "../k_term.h"
#include <stddef.h>

static uint16_t k_gfx_vga_ext_reg_ports[][2] = 
{ 
    [K_GFX_VGA_EXT_REG_MISC][0] = K_GFX_VGA_EXT_REG_MISC_READ_PORT,
    [K_GFX_VGA_EXT_REG_MISC][1] = K_GFX_VGA_EXT_REG_MISC_WRITE_PORT,

    [K_GFX_VGA_EXT_REG_FEAT_CTRLM][0] = K_GFX_VGA_EXT_REG_FEAT_CTRLM_READ_PORT,
    [K_GFX_VGA_EXT_REG_FEAT_CTRLM][1] = K_GFX_VGA_EXT_REG_FEAT_CTRLM_WRITE_PORT,

    [K_GFX_VGA_EXT_REG_FEAT_CTRLC][0] = K_GFX_VGA_EXT_REG_FEAT_CTRLC_READ_PORT,
    [K_GFX_VGA_EXT_REG_FEAT_CTRLC][1] = K_GFX_VGA_EXT_REG_FEAT_CTRLC_WRITE_PORT,

    [K_GFX_VGA_EXT_REG_INPUT_STATUS0][0] = K_GFX_VGA_EXT_REG_INPUT_STATUS0_READ_PORT,
    [K_GFX_VGA_EXT_REG_INPUT_STATUS0][1] = 0x0000,

    [K_GFX_VGA_EXT_REG_INPUT_STATUS1M][0] = K_GFX_VGA_EXT_REG_INPUT_STATUS1M_READ_PORT,
    [K_GFX_VGA_EXT_REG_INPUT_STATUS1M][1] = 0x0000,

    [K_GFX_VGA_EXT_REG_INPUT_STATUS1C][0] = K_GFX_VGA_EXT_REG_INPUT_STATUS1C_READ_PORT,
    [K_GFX_VGA_EXT_REG_INPUT_STATUS1C][1] = 0x0000 
};

static uint16_t *k_gfx_vga_mem_maps[] = {(uint16_t *)0xa0000, (uint16_t *)0xa0000, (uint16_t *)0xb0000, (uint16_t *)0xb8000};

uint16_t *k_gfx_vga_cur_mem_map = NULL;

uint32_t k_gfx_vga_width = 0;
uint32_t k_gfx_vga_height = 0;
uint32_t k_gfx_vga_cursor_col = 0;
uint32_t k_gfx_vga_cursor_row = 0;

void k_gfx_InitVga()
{
    k_gfx_SetVgaMemMap(K_GFX_VGA_MEM_MAP0);
    // k_gfx_SetVgaGfxMode(K_GFX_VGA_GFX_MODE_GRAPHIC);

    // k_gfx_WriteVgaSeqReg(K_GFX_VGA_SEQ_REG_MAP_MASK, 0x0f);

    // uint8_t reg_value = k_gfx_ReadVgaSeqReg(K_GFX_VGA_SEQ_REG_MEMORY_MODE);
    // reg_value |= K_GFX_VGA_SEQ_MEMORY_MODE_REG_OE_DISABLE;
    // k_gfx_WriteVgaSeqReg(K_GFX_VGA_SEQ_REG_MEMORY_MODE, reg_value);

    // uint8_t reg_value = k_gfx_ReadVgaGfxReg(K_GFX_VGA_GFX_REG_MISC);
    // reg_value |= K_GFX_VGA_GFX_MISC_REG_FLAG_CHAIN_OE_ENABLE;
    // k_gfx_WriteVgaGfxReg(K_GFX_VGA_GFX_REG_MISC, reg_value);

    // k_gfx_FillSquare(0, 0, 0, 0);
    k_gfx_SetVgaGfxMode(K_GFX_VGA_GFX_MODE_ALPHANUMERIC);
}

void k_gfx_WriteVgaGfxReg(uint8_t reg, uint8_t value)
{
    if(reg < K_GFX_VGA_GFX_REG_LAST)
    {
        k_cpu_OutB(reg, K_GFX_VGA_GFX_REG_ADDR_PORT);
        k_cpu_OutB(value, K_GFX_VGA_GFX_REG_DATA_PORT);
    }
}

uint8_t k_gfx_ReadVgaGfxReg(uint8_t reg)
{
    if(reg < K_GFX_VGA_GFX_REG_LAST)
    {
        k_cpu_OutB(reg, K_GFX_VGA_GFX_REG_ADDR_PORT);
        return k_cpu_InB(K_GFX_VGA_GFX_REG_DATA_PORT);
    }

    return 0;
}

void k_gfx_WriteVgaCrtReg(uint8_t reg, uint8_t value)
{
    if(reg < K_GFX_VGA_CRT_REG_LAST)
    {
        uint16_t port = K_GFX_VGA_CRT_REG_ADDR_PORT0;

        if(k_gfx_ReadVgaExtReg(K_GFX_VGA_EXT_REG_MISC) & K_GFX_VGA_EXT_MISC_REG_FLAG_IO_ADDR_SEL)
        {
            /* address port is 0x3?4 and data port is 0x3?5. The value of ? depends on bit 0
            of the external misc register. If it's 0, ? = B, otherwise it's D. By adding 0x20 here
            we go from B to D in case the bit is set. */
            port += 0x20;
        }

        k_cpu_OutB(reg, port);
        k_cpu_OutB(value, port + 1);
    }
}

uint8_t k_gfx_ReadVgaCrtReg(uint8_t reg)
{
    if(reg < K_GFX_VGA_CRT_REG_LAST)
    {
        uint16_t port = K_GFX_VGA_CRT_REG_ADDR_PORT0;
        
        if(k_gfx_ReadVgaExtReg(K_GFX_VGA_EXT_REG_MISC) & K_GFX_VGA_EXT_MISC_REG_FLAG_IO_ADDR_SEL)
        {
            /* address port is 0x3?4 and data port is 0x3?5. The value of ? depends on bit 0
            of the external misc register. If it's 0, ? = B, otherwise it's D. By adding 0x20 here
            we go from B to D in case the bit is set. */
            port += 0x20;
        }

        k_cpu_OutB(reg, port);
        return k_cpu_InB(port + 1);
    }

    return 0;
}

void k_gfx_WriteVgaSeqReg(uint8_t reg, uint8_t value)
{
    if(reg < K_GFX_VGA_SEQ_REG_LAST)
    {
        k_cpu_OutB(reg, K_GFX_VGA_SEQ_REG_ADDRESS_PORT);
        k_cpu_OutB(value, K_GFX_VGA_SEQ_REG_DATA_PORT);
    }
}

uint8_t k_gfx_ReadVgaSeqReg(uint8_t reg)
{
    if(reg < K_GFX_VGA_SEQ_REG_LAST)
    {
        k_cpu_OutB(reg, K_GFX_VGA_SEQ_REG_ADDRESS_PORT);
        return k_cpu_InB(K_GFX_VGA_SEQ_REG_DATA_PORT);
    }

    return 0;
}

void k_gfx_WriteVgaExtReg(uint8_t reg, uint8_t value)
{
    if(reg < K_GFX_VGA_EXT_REG_LAST && k_gfx_vga_ext_reg_ports[reg][1])
    {
        k_cpu_OutB(value, k_gfx_vga_ext_reg_ports[reg][1]);
    }
}

uint8_t k_gfx_ReadVgaExtReg(uint8_t reg)
{
    if(reg < K_GFX_VGA_EXT_REG_LAST)
    {
        if(reg == K_GFX_VGA_EXT_REG_INPUT_STATUS1 || reg == K_GFX_VGA_EXT_REG_FEAT_CTRL)
        {
            /* this will select the monochrome port of the register */
            reg++;

            if(k_gfx_ReadVgaExtReg(K_GFX_VGA_EXT_REG_MISC) & K_GFX_VGA_EXT_MISC_REG_FLAG_IO_ADDR_SEL)
            {
                /* this will select the color port of the register */
                reg++;
            }
        }

        return k_cpu_InB(k_gfx_vga_ext_reg_ports[reg][0]);
    }

    return 0;
}

/*
=============================================================================================
=============================================================================================
=============================================================================================
*/

void k_gfx_SetVgaMemMap(uint32_t mem_map)
{
    if(mem_map <= K_GFX_VGA_MEM_MAP3)
    {
        uint8_t reg_value = k_gfx_ReadVgaGfxReg(K_GFX_VGA_GFX_REG_MISC);
        reg_value = (reg_value & (~K_GFX_VGA_MEM_MAP_SEL_MASK)) | (mem_map << K_GFX_VGA_MEM_MAP_SEL_SHIFT);
        k_gfx_WriteVgaGfxReg(K_GFX_VGA_GFX_REG_MISC, reg_value);
        k_gfx_vga_cur_mem_map = k_gfx_vga_mem_maps[mem_map];
    }
}

void k_gfx_SetVgaCursorPos(uint32_t column, uint32_t row)
{
    (void)column;
    (void)row;
}


void k_gfx_ToggleVgaCursor(uint32_t enable)
{
    uint8_t reg_value = k_gfx_ReadVgaCrtReg(K_GFX_VGA_CRT_REG_CURSOR_START);

    if(!enable)
    {
        reg_value |= K_GFX_VGA_CRT_CURSOR_START_REG_FLAG_CURSOR_DISABLE;
    }
    else
    {
        reg_value &= ~K_GFX_VGA_CRT_CURSOR_START_REG_FLAG_CURSOR_DISABLE;
    }

    k_gfx_WriteVgaCrtReg(K_GFX_VGA_CRT_REG_CURSOR_START, reg_value);
}

void k_gfx_SetVgaGfxMode(uint32_t mode)
{
    // (void)mode;
    uint8_t misc_value = k_gfx_ReadVgaGfxReg(K_GFX_VGA_GFX_REG_MISC);
    uint8_t map_mask_value = 0;

    if(mode == K_GFX_VGA_GFX_MODE_GRAPHIC)
    {
        misc_value |= K_GFX_VGA_GFX_MISC_REG_FLAG_ALPHA_DISABLE;
        map_mask_value = 0x0f;
    }
    else
    {
        misc_value &= ~K_GFX_VGA_GFX_MISC_REG_FLAG_ALPHA_DISABLE;
        map_mask_value = 0x03;
    }

    k_gfx_WriteVgaGfxReg(K_GFX_VGA_GFX_REG_MISC, misc_value);
    k_gfx_WriteVgaSeqReg(K_GFX_VGA_SEQ_REG_MAP_MASK, map_mask_value);
}

void k_gfx_FillSquare(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    uint8_t *framebuffer = (uint8_t *)k_gfx_vga_cur_mem_map;


    for(uint32_t index = 0; index < 0xffff; index++)
    {
        framebuffer[index] = 0xf;
    }
}