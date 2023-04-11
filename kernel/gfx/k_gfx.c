#include "k_gfx.h"

void k_gfx_Init()
{
    __asm__ volatile
    (
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );
    k_gfx_InitVga();
}