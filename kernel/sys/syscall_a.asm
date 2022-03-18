.intel_syntax noprefix
.code32

.section .text
.include "proc/defs_a.inc"
.include "sys/defs_a.inc"

.global k_sys_SysCall
k_sys_SysCall:
    push ebp
    mov ebp, esp
    /* alloc temp var for return address */
    push eax
    call k_proc_PushThreadRegs
    mov eax, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebp + 12]
    mov ecx, dword ptr [ebp + 16]
    mov edx, dword ptr [ebp + 20]
    int 0x45
    /* store return address on temp var */
    mov dword ptr [ebp - 4], eax
    call k_proc_PopThreadRegs
    /* pop return address into eax, since it got overwritten by k_proc_PopThreadRegs */
    pop eax
    pop ebp
    ret

.global k_sys_SysCall_a
k_sys_SysCall_a:
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
    mov eax, dword ptr[k_proc_core_state + k_proc_core_state_current_thread]
    mov eax, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    add esp, eax
    /* add esp, dword ptr [eax + k_proc_thread_kernel_stack_offset] */

    /* address of the syscall arg buffer */
    lea ecx, [esp + 4]
    push ecx
    call k_sys_DispatchSysCall
    pop ecx

    /* process page map */
    pop ebx
    mov ecx, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
    mov ecx, dword ptr [ecx + k_proc_thread_kernel_stack_offset]
    /* switch to process space stack */
    sub esp, ecx
    /* switch back to process space */
    mov cr3, ebx
    add esp, k_sys_syscall_args_size
    iret
