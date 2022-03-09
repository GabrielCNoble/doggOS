#include "shell.h"
#include "term.h"
#include <stddef.h>

uintptr_t k_sys_ShellMain(void *data)
{
    char keyboard_buffer[512];
    k_sys_TerminalPrintf("Initializing root shell...\n");

    // uint32_t *p = NULL;
    // *p = 6;

    // uint32_t i = 1 / 0;
    // asm volatile ("mov ax, 0\n mov ss, ax\n");
    while(1)
    {
        uint32_t char_count = k_sys_TerminalReadLine(keyboard_buffer, 512);

        if(char_count)
        {

        }
    }
}