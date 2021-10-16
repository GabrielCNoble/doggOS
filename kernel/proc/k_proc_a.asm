.intel_syntax noprefix
.code32

.section .text

/* .global k_proc_EnterCriticalSection
k_proc_EnterCriticalSection:
    mov eax, dword ptr [esp + 4]
    push eax
    call k_atm_Spinlock
    cli
    pop eax
    ret

.global k_proc_ExitCriticalSection
k_proc_ExitCriticalSection:
    mov eax, dword ptr [esp + 4]
    push eax
    call k_atm_SpinUnlock
    pop eax
    sti
    ret */

.global k_proc_SwitchToThread
k_proc_SwitchToThread:
    mov eax, dword ptr [esp + 4]
    mov dword ptr [k_proc_next_thread], eax
    int 38
    ret

.global k_proc_PreemptCurrentThread
k_proc_PreemptCurrentThread:
    /* save gprs */
    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
    push ebp
    /* save page dir */
    mov eax, cr3
    push eax
    /* save sregs */
    xor eax, eax
    mov ax, ds
    push eax
    mov ax, es
    push eax
    mov ax, fs
    push eax

    /* save esp from prev thread */
    mov eax, dword ptr [k_proc_current_thread]
    mov dword ptr [eax + 32], esp

    /* load esp for next thread */
    mov eax, dword ptr [k_proc_next_thread]
    cmp eax, 0
    jnz _valid_next_thread
        /* next thread pointer is null, which means we'll be switching to the
        scheduler thread */
        mov eax, offset k_proc_scheduler_thread
    _valid_next_thread:
    mov dword ptr [k_proc_current_thread], eax
    mov esp, dword ptr [eax + 32]
    mov ebx, dword ptr [eax + 4]
    mov dword ptr [k_proc_current_process], ebx

    /* store the start of the switch stack segment into the loaded 
    tss. This is just useful for threads running out of ring 0 */
    mov ebx, dword ptr [eax + 28]
    mov eax, dword ptr [k_proc_tss]
    mov dword ptr [eax + 4], ebx

    /* clear the next thread pointer once we're done with it, to make
    sure that in the worst case scenario a call to this interrupt handler
    takes the cpu back to the scheduler */
    xor eax, eax
    mov dword ptr [k_proc_next_thread], eax

    /* restore sregs */
    pop eax
    mov fs, ax
    pop eax
    mov es, ax
    pop eax
    mov ds, ax
    /* restore page dir */
    pop eax
    /* restore gprs */
    pop ebp
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    
    iret
    