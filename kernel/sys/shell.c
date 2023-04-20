#include "shell.h"
#include "term.h"
#include "syscall.h"
#include "../io.h"
#include "../rt/alloc.h"
#include "../rt/mem.h"
#include "../rt/string.h"
#include "../proc/proc.h"
#include "../proc/defs.h"
#include "../dsk/dsk.h"
#include "../fs/fs.h"
// #include "../dev/pci/piix3/ide.h"
#include "../fs/pup.h"
// #include "../proc/elf.h"
#include <stddef.h>

void k_sys_Help()
{
    k_sys_TerminalPrintf("Available commands:\n");
    k_sys_TerminalPrintf("    help: runs this command\n");
    k_sys_TerminalPrintf("    clear: clears the screen\n");
    k_sys_TerminalPrintf("    crash: crashes the system\n");
    k_sys_TerminalPrintf("    cd: changes directory\n");
    k_sys_TerminalPrintf("    dir: lists contents of directory\n");
    // k_sys_TerminalPrintf("    syscall: executes test syscall\n");
    // k_sys_TerminalPrintf("    exp: launches expression parser program\n");
    // k_sys_TerminalPrintf("    derp: launches derp program\n");
    k_sys_TerminalPrintf("    disks: prints information about detected disks\n");
    // k_sys_TerminalPrintf("    fs_test: does a test disk read through the file system\n");
}

extern void *k_kernel_end2;
extern void *k_share_start;
extern void *k_share_end;

// extern struct k_io_stream_t *k_PIIX3_IDE_stream;
// extern struct k_dev_disk_t *k_PIIX3_IDE_disk;
// extern struct k_dsk_disk_t *k_dsk_disks;
extern struct k_dev_device_t *k_dev_devices;

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

    k_sys_TerminalPrintf("Phew... (%s)\n", keyboard_buffer);
}

uintptr_t k_sys_ShellMain(void *data)
{
    char keyboard_buffer[512];
    char current_path[512];
    
    // k_dev_StartDevices();    
    k_sys_TerminalSetColor(K_SYS_TERM_COLOR_WHITE, K_SYS_TERM_COLOR_BLACK);
    k_sys_TerminalPrintf("Initializing root shell...\n");
    // k_sys_SysCall(K_SYS_SYSCALL_TEST_CALL, 0, 1, 2);

    struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    current_process->terminal = k_io_AllocStream();
    k_io_UnblockStream(current_process->terminal);

    // k_sys_TerminalPrintf("blah...\n");

    // char buffer[] = "abcdefgh";

    // k_rt_SetBytes(buffer, 6, 'z');

    // k_sys_TerminalPrintf("%s\n", buffer);

    struct k_dev_device_t *device = k_dev_devices;
    while(device)
    {
        if(device->device_type == K_DEV_DEVICE_TYPE_DISK)
        {
            break;
        }
        device = device->next_device;
    }

    struct k_fs_part_t partition = {.first_block = 188, .block_count = 8192, .disk = (struct k_dev_disk_t *)device};
    
    struct k_fs_vol_t *pup_volume = k_fs_MountVolume(&partition);
    
    // struct k_fs_pup_link_t cur_dir_node;    
    struct k_fs_pup_link_t cur_dir_node = k_fs_PupFindNode(pup_volume, "/", K_FS_PUP_NULL_LINK);
    k_fs_PupGetPathToNode(pup_volume, cur_dir_node, current_path, sizeof(current_path));
    // k_sys_TerminalClear();
    

    while(1)
    {
        k_sys_TerminalPrintf("[root > %s ]: ", current_path);        
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
        // else if(!k_rt_StrCmp(keyboard_buffer, "syscall"))
        // {
        //     uint32_t status = k_sys_SysCall(K_SYS_SYSCALL_TEST_CALL, 0xff, 0xb00b1e5, 0xaaaaaaaa, 0);
        //     k_sys_TerminalPrintf("syscall returned with status %x\n", status);
        // }
        // else if(!k_rt_StrCmp(keyboard_buffer, "exp"))
        // {
        //     // void *image_buffer = k_rt_Malloc(0xffff, 4);
        //     // k_PIIX3_IDE_Read(140, 32);
        //     // for(uint32_t x = 0; x < 0x1ffffff; x++);
        //     // k_io_ReadStream(k_PIIX3_IDE_stream, 0, image_buffer, 0xffff);
        //     // 
        //     // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
        //     // struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
        //     // uintptr_t return_value;
        //     // k_proc_WaitProcess(process, &return_value);
        //     // k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
        //     // k_rt_Free(image_buffer);
        // 
        //     void *image_buffer = k_rt_Malloc(0xffff, 4);
        //     k_dsk_Read(k_PIIX3_IDE_disk, 170 * 512, 32 * 512, image_buffer);
        //     // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
        //     struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
        //     uintptr_t return_value;
        //     k_proc_WaitProcess(process, &return_value);
        //     k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
        //     k_rt_Free(image_buffer);
        // }
        // else if(!k_rt_StrCmp(keyboard_buffer, "derp"))
        // {
        //     void *image_buffer = k_rt_Malloc(0xffff, 4);
        //     k_dsk_Read(k_PIIX3_IDE_disk, 210 * 512, 32 * 512, image_buffer);
        //     // // struct k_proc_process_t *process = k_proc_LaunchProcess("./blah.elf", NULL);
        //     struct k_proc_process_t *process = k_proc_CreateProcess(image_buffer, NULL, NULL);
        //     uintptr_t return_value;
        //     k_proc_WaitProcess(process, &return_value);
        //     k_sys_TerminalPrintf("process %x returned with value %x\n", process, return_value);
        //     k_rt_Free(image_buffer);
        // }
        // else if(!k_rt_StrCmp(keyboard_buffer, "disks"))
        // {
        //     struct k_dsk_disk_t *disk = k_dsk_disks;
        //     while(disk)
        //     {
        //         k_sys_TerminalPrintf("disk: %x, block size = %d, block count = %d\n", disk, disk->block_size, disk->block_count);
        //         disk = disk->next;
        //     }
        // }
        // else if(!k_rt_StrCmp(keyboard_buffer, "dir"))
        // {
        //     struct k_fs_pup_dirlist_t *dir_list = k_fs_PupGetNodeDirList(pup_volume, "", cur_dir_node);
        //     k_sys_TerminalPrintf("  Contents of directory %s\n", current_path);
        //     if(dir_list)
        //     {
        //         for(uint32_t entry_index = 0; entry_index < dir_list->used_count; entry_index++)
        //         {
        //             struct k_fs_pup_dirent_t *entry = dir_list->entries + entry_index;
                    
        //             if(!entry->node.link)
        //             {
        //                 break;
        //             }
                    
        //             k_sys_TerminalPrintf("      %s\n", entry->name);
        //         }
                
        //         k_rt_Free(dir_list);
        //     }
            
        //     k_sys_TerminalPrintf("\n");
        // }
        else if(k_rt_StrStr(keyboard_buffer, "cd") == &keyboard_buffer[0])
        {
            uint32_t index = 0;
            
            while(keyboard_buffer[index] != ' ' && keyboard_buffer[index] != '\0')
            {
                index++;
            }
        
            
            if(keyboard_buffer[index])
            {
                struct k_fs_pup_link_t node = k_fs_PupFindNode(pup_volume, keyboard_buffer + index, cur_dir_node);
                
                if(node.link)
                {
                    cur_dir_node = node;
                    // current_path[0] = '\0';
                    k_fs_PupGetPathToNode(pup_volume, cur_dir_node, current_path, sizeof(current_path));
                }
                else
                {
                    k_sys_TerminalPrintf("%s: no such file or directory\n", keyboard_buffer + index);
                }
            }
        }
        // else if(k_rt_StrStr(keyboard_buffer, "mkdir") == &keyboard_buffer[0])
        // {
        //     struct k_fs_pup_link_t node = k_fs_PupAllocNode(pup_volume, K_FS_PUP_NODE_TYPE_DIR);
        //     k_sys_TerminalPrintf("node: %x\n", (uint32_t)node.link);
        // }
        else if(keyboard_buffer[0] == '\0')
        {

        }
        else
        {
            k_sys_TerminalPrintf("ERROR: unknown command '%s'\n", keyboard_buffer);
        }
    }

    return 0;
}
