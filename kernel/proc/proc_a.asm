.intel_syntax noprefix
.code32

.section .text
.include "proc/defs_a.inc"

.global k_proc_SetupUserStack_a
k_proc_SetupUserStack_a:
    /* thread init */
    mov eax, dword ptr [esp + 8]
    mov ebx, dword ptr [eax + k_proc_thread_init_user_stack]
    sub ebx, 12

    /* setup a call to k_proc_StartThread */
    mov ecx, dword ptr [eax + k_proc_thread_init_user_data]
    mov dword ptr [ebx + 8], ecx
    mov ecx, dword ptr [eax + k_proc_thread_init_entry_point]
    mov dword ptr [ebx + 4], ecx
    /* k_proc_StartThread won't ever return */
    mov dword ptr [ebx], 0

    /* call k_proc_StartUserThread_a */
    int 39

.global k_proc_SetupThreadStack_a
k_proc_SetupThreadStack_a:
    mov eax, dword ptr [esp + 0]

.global k_proc_StartUserThread_a
k_proc_StartUserThread_a:
    /* eflags */
    mov eax, dword ptr [esp + 8]
    /* we need to make room for the user stack segment and stack pointer */
    sub esp, 8
    mov dword ptr [esp], offset k_proc_StartUserThread
    /* ring 3 code segment */
    mov dword ptr [esp + 4], k_proc_r3_code_seg
    /* eflags */
    mov dword ptr [esp + 8], eax
    /* user stack */
    mov dword ptr [esp + 12], ebx
    /* ring 3 stack segment */
    mov dword ptr [esp + 16], k_proc_r3_data_seg
    iret
