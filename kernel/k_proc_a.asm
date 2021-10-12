.intel_syntax noprefix
.code32

.section .text

.global k_proc_SwitchToThread
k_proc_SwitchToThread:
    mov eax, dword ptr [esp + 4]
    mov dword ptr [k_proc_next_thread], eax
    int 38
    ret

.global k_proc_PreemptCurrentThread
k_proc_PreemptCurrentThread:
    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
    push ebp
    mov eax, cr3
    push eax
    xor eax, eax
    mov ax, ds
    push eax
    mov ax, es
    push eax
    mov ax, fs
    push eax

    /* call k_apic_SignalFixedInterruptHandled */

    /* save esp from prev thread */
    mov eax, dword ptr [k_proc_active_thread]
    mov dword ptr [eax + 28], esp

    /* load esp for next thread */
    mov eax, dword ptr [k_proc_next_thread]
    cmp eax, 0
    jnz _valid_next_thread
        mov eax, offset k_proc_scheduler_thread
    _valid_next_thread:
    mov dword ptr [k_proc_active_thread], eax
    mov esp, dword ptr [eax + 28]

    /* store the start of the switch stack segment into the loaded 
    tss. This is just useful for threads running out of ring 0 */
    mov ebx, dword ptr [eax + 24]
    mov eax, dword ptr [k_proc_tss]
    mov dword ptr [eax + 4], ebx

    xor eax, eax
    mov dword ptr [k_proc_next_thread], eax

    pop eax
    mov fs, ax
    pop eax
    mov es, ax
    pop eax
    mov ds, ax
    pop eax
    pop ebp
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    
    iret
    