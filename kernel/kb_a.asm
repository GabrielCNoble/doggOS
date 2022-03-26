.intel_syntax noprefix
.code32

.include "proc/defs_a.inc"

.section .text
.global k_kb_KeyboardHandler_a
k_kb_KeyboardHandler_a:
    /* cli
    hlt*/
    nop
    nop 
    call k_proc_PushThreadRegs
    mov eax, cr3
    push eax
    mov eax, k_proc_shared_data_address
    mov eax, dword ptr [eax + k_proc_shared_data_kernel_pmap]
    mov cr3, eax
    mov eax, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
    add esp, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    call k_kb_KeyboardHandler
    pop ebx
    mov eax, dword ptr [k_proc_core_state + k_proc_core_state_current_thread]
    sub esp, dword ptr [eax + k_proc_thread_kernel_stack_offset]
    mov cr3, ebx
    call k_proc_PopThreadRegs
    iret