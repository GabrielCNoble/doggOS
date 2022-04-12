.code32
.intel_syntax noprefix
.section .text

.include "proc/defs_a.inc"

.global k_proc_ExitThreadContext
k_proc_ExitThreadContext:
    /* gprs */
    push ebx
    mov ebx, dword ptr [esp + 4]
    /* store eax at the return address */
    mov dword ptr [esp + 4], eax
    push ecx
    push edx
    push edi
    push esi
    push ebp

    /* sregs */
    xor eax, eax
    mov ax, ds
    push eax
    mov ax, es
    push eax
    mov ax, fs
    push eax

    /* page map */
    mov eax, cr3
    push eax

    /* restore return address */
    push ebx

    mov ebx, dword ptr [k_proc_shared_data_address + k_proc_shared_data_kernel_pmap]
    cmp ebx, eax
    je _skip_enter_kernel_space
        mov cr3, ebx
        mov ebx, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
        add esp, dword ptr [ebx + k_proc_thread_kernel_stack_offset]
    _skip_enter_kernel_space:
    /* restore clobbered regs */
    mov eax, dword ptr [esp + 44]
    mov ebx, dword ptr [esp + 40]
    ret

.global k_proc_EnterThreadContext
k_proc_EnterThreadContext:
    /* save return address */
    pop eax

    /* page map */
    pop ecx
    cmp ecx, dword ptr [k_proc_shared_data_address + k_proc_shared_data_kernel_pmap]
    je _skip_exit_kernel_space
        mov ebx, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
        sub esp, dword ptr [ebx + k_proc_thread_kernel_stack_offset]
        mov cr3, ecx
    _skip_exit_kernel_space:

    /* sregs */
    pop ebx
    mov fs, bx
    pop ebx
    mov es, bx
    pop ebx
    mov ds, bx

    /* gprs */
    pop ebp
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx

    /* restore return address */
    mov dword ptr [esp - 4], eax
    pop eax
    push dword ptr [esp - 8]

    ret
