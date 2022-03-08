#include "shell.h"
#include "term.h"

uintptr_t k_sys_ShellMain(void *data)
{
    char keyboard_buffer[512];
    k_sys_TerminalPrintf("Initializing root shell...\n");
    while(1)
    {
        uint32_t char_count = k_sys_TerminalReadLine(keyboard_buffer, 512);

        if(char_count)
        {

        }
    }
}