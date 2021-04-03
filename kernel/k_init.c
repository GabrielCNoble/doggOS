#include "k_mem.h"
#include "k_term.h"
#include "k_int.h"

void k_init()
{
    k_mem_init();
    k_int_init();
    k_term_init();
    asm volatile
    (
        "jmp 0x10:0x00400000\n"
    );
}