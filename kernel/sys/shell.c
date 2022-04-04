#include "shell.h"
#include "term.h"
#include "syscall.h"
#include "../io.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../proc/proc.h"
#include "../proc/defs.h"
#include "../dsk/dsk.h"
#include "../dev/pci/piix3/ide.h"
// #include "../proc/elf.h"
#include <stddef.h>

void k_sys_Help()
{
    k_sys_TerminalPrintf("Available commands:\n");
    k_sys_TerminalPrintf("    help: runs this command\n");
    k_sys_TerminalPrintf("    clear: clears the screen\n");
    k_sys_TerminalPrintf("    crash: crashes the system\n");
    k_sys_TerminalPrintf("    syscall: executes test syscall\n");
    k_sys_TerminalPrintf("    exp: launches expression parser program\n");
    k_sys_TerminalPrintf("    derp: launches derp program\n");
    // k_sys_TerminalPrintf("    test_read: does a test disk read\n");
}

extern void *k_kernel_end2;
extern void *k_share_start;
extern void *k_share_end;

// extern struct k_io_stream_t *k_PIIX3_IDE_stream;
extern struct k_dsk_disk_t *k_PIIX3_IDE_disk;

void k_sys_Crash()
{
    char keyboard_buffer[8];

    k_sys_TerminalPrintf("Are you sure you want to crash the machine? (y/n) ");
    k_sys_TerminalReadLine(keyboard_buffer, 8);

    if(!k_rt_StrCmp(keyboard_buffer, "y") || !k_rt_StrCmp(keyboard_buffer, "Y"))
    {
        // uintptr_t *crash = (uint32_t *)0x1234567;
        // *crash = 0xb00b1e5;

        k_sys_SysCall(K_SYS_SYSCALL_CRASH, 0, 0, 0, 0);
    }

    k_sys_TerminalPrintf("Phew...\n");
}

uintptr_t k_sys_ShellMain(void *data)
{
    char keyboard_buffer[512];
    k_sys_TerminalClear();
    k_sys_TerminalPrintf("Initializing root shell...\n");
    // k_sys_SysCall(K_SYS_SYSCALL_TEST_CALL, 0, 1, 2);

    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    current_process->terminal = k_io_AllocStream();
    k_io_UnblockStream(current_process->terminal);

    // k_sys_TerminalPrintf("%x %x\n", &k_share_start, &k_share_end);
    // struct k_proc_process_t *process = k_proc_CreateProcess(&k_kernel_end2, 0x1000);

    while(1)
    {
        k_sys_TerminalPrintf("[root]:");
        k_sys_TerminalReadLine(keyboard_buffer, 512);
        // {
        if(!k_rt_StrCmp(keyboard_buffer, "help"))
        {
            k_sys_Help();
        }
        else if(!k_rt_StrCmp(keyboard_buffer, "clear"))
        {
            k_sys_TerminalClear();
        }
        else if(!k_rt_StrCmp(keyboard_buffer, "crash"))
        {
            k_sys_Crash();
        }
        else if(!k_rt_StrCmp(keyboard_buffer, "syscall"))
        {
            uint32_t status = k_sys_SysCall(K_SYS_SYSCALL_TEST_CALL, 0xff, 0xb00b1e5, 0xaaaaaaaa, 0);
            k_sys_TerminalPrintf("syscall returned with status %x\n", status);
        }
        else if(!k_rt_StrCmp(keyboard_buffer, "exp"))
        {
            // void *image_buffer = k_rt_Malloc(0xffff, 4);
            // k_PIIX3_IDE_Read(140, 32);
            // for(uint32_t x = 0; x < 0x1ffffff; x++);
            // k_io_ReadStream(k_PIIX3_IDE_stream, 0, image_buffer, 0xffff);
            // 
            // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
            // struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
            // uintptr_t return_value;
            // k_proc_WaitProcess(process, &return_value);
            // k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
            // k_rt_Free(image_buffer);
            
            void *image_buffer = k_rt_Malloc(0xffff, 4);
            k_dsk_Read(k_PIIX3_IDE_disk, 140 * 512, 32 * 512, image_buffer);
            // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
            struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
            uintptr_t return_value;
            k_proc_WaitProcess(process, &return_value);
            k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
            k_rt_Free(image_buffer);
        }
        else if(!k_rt_StrCmp(keyboard_buffer, "derp"))
        {
            void *image_buffer = k_rt_Malloc(0xffff, 4);
            k_dsk_Read(k_PIIX3_IDE_disk, 170 * 512, 32 * 512, image_buffer);
            // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
            struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
            uintptr_t return_value;
            k_proc_WaitProcess(process, &return_value);
            k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
            k_rt_Free(image_buffer);
        }
        // else if(!k_rt_StrCmp(keyboard_buffer, "test_read"))
        // {
        //     k_PIIX3_IDE_Read(142, 1);
        //     for(uint32_t x = 0; x < 0xffffff; x++);
        //     k_io_ReadStream(k_PIIX3_IDE_stream, keyboard_buffer, K_PROC_ELF_IDENT);
        // 
        //     for(uint32_t ident = 0; ident < K_PROC_ELF_IDENT; ident++)
        //     {
        //         k_sys_TerminalPrintf("%x ", keyboard_buffer[ident]);
        //     }
        //     k_sys_TerminalPrintf("\n");
        // }
        else if(keyboard_buffer[0] == '\0')
        {

        }
        else
        {
            k_sys_TerminalPrintf("ERROR: unknown command '%s'\n", keyboard_buffer);
        }
        // }
    }

    return 0;
}
