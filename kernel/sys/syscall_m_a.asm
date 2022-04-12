.intel_syntax noprefix
.code32

.section .text

.include "proc/defs_a.inc"
.include "sys/defs_a.inc"

.global k_sys_SysCall_a
k_sys_SysCall_a:
    push ebp
    mov ebp, esp
    sub esp, k_sys_syscall_args_size
    /* syscall arg buffer */
    mov dword ptr [esp + 0], eax
    mov dword ptr [esp + 4], ebx
    mov dword ptr [esp + 8], ecx
    mov dword ptr [esp + 12], edx

    /* switch to kernel space */
    mov eax, k_proc_shared_data_address
    mov eax, dword ptr [eax + k_proc_shared_data_kernel_pmap]
    mov cr3, eax

    /* switch to kernel space stack */
    mov eax, dword ptr[k_proc_core_state + k_proc_core_state_current_thread]
    mov eax, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    add esp, eax

    sti

    /* address of the syscall arg buffer */
    mov ecx, esp
    push ecx
    call k_sys_DispatchSysCall
    pop ecx

    cli

    mov ecx, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
    mov ebx, dword ptr [ecx + k_proc_thread_process]

    mov ebx, dword ptr [ebx + k_proc_process_page_map]
    mov dword ptr [ecx + k_proc_thread_current_pmap], ebx

    mov ecx, dword ptr [ecx + k_proc_thread_kernel_stack_offset]
    /* switch to process space stack */
    sub esp, ecx
    /* switch back to process space */
    mov cr3, ebx
    add esp, k_sys_syscall_args_size
    pop ebp
    iret
