.intel_syntax noprefix
.code32

.section .text
.include "proc/defs_a.inc"
.include "sys/defs_a.inc"

.global k_sys_SysCall
k_sys_SysCall:
    call k_proc_PushThreadRegs
    mov eax, dword ptr [esp + 4]
    mov ebx, dword ptr [esp + 8]
    mov ecx, dword ptr [esp + 12]
    mov edx, dword ptr [esp + 16]
    int 69
    call k_proc_PopThreadRegs
    ret

.global k_sys_SysCall_a
k_sys_SysCall_a:
    call k_proc_PushThreadRegs

    /* syscall arg buffer */
    sub esp, k_sys_syscall_args_size
    mov dword ptr [esp + 0], eax
    mov dword ptr [esp + 4], ebx
    mov dword ptr [esp + 8], ecx
    mov dword ptr [esp + 12], edx

    /* If the thread gets preempted while executing the syscall, the kernel
    pmap will be stored in the thread struct, overwritting the process pmap.
    To allow restoring the process pmap if that happens we store it here
    before switching to the kernel address space. */
    mov eax, cr3
    push eax
    /* switch to kernel space */
    mov eax, k_proc_page_map
    mov cr3, eax

    /* switch to kernel space stack */
    mov eax, offset k_proc_core_state + k_proc_core_state_current_thread
    add esp, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    /* address of the syscall arg buffer */
    lea eax, [esp + 4]
    push eax
    call k_sys_DispatchSysCall
    pop eax

    /* process page map */
    pop ebx
    mov eax, offset k_proc_core_state + k_proc_core_state_current_thread
    /* switch to process space stack */
    sub esp, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    /* switch back to process space */
    mov cr3, ebx
    add esp, k_sys_syscall_args_size
    /* mov dword ptr [esp - k_proc_syscall_args_size], eax */
    call k_proc_PopThreadRegs
    iret
