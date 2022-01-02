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
    /* save sregs */
    xor eax, eax
    mov ax, ds
    push eax
    mov ax, es
    push eax
    mov ax, fs
    push eax
    mov edx, cr3
    mov eax, dword ptr [k_proc_current_thread]
    mov ebx, dword ptr [k_proc_next_thread]
    cmp ebx, 0
    jnz _valid_next_thread
    /* next thread is NULL, which means we're going back to the scheduler. For that to
    work, we'll need to load the kernel page dir before trying to access anything other
    than the scheduler thread state */
    mov ebx, offset k_proc_scheduler_thread
    mov ecx, dword ptr [ebx + 40]
    /* we're rolling with the kernel page dir now, so accessing the next thread and the tss 
    pointers is safe */
    mov cr3, ecx
    _valid_next_thread:
    /* save page dir of current thread */
    mov dword ptr [eax + 40], edx
    /* save stack top of current thread */
    mov dword ptr [eax + 36], esp

    /* current thread stuff done, now we setup next thread stuff. */
    mov dword ptr [k_proc_current_thread], ebx
    /* store the start of the switch stack segment into the loaded tss. This is just useful 
    for threads running out of ring 0 */
    mov ecx, dword ptr [ebx + 32]
    mov eax, dword ptr [k_proc_tss]
    mov dword ptr [eax + 4], ecx

    /* clear the next thread pointer once we're done with it, to make sure that in the worst 
    case scenario a call to this interrupt handler takes the cpu back to the scheduler */
    xor eax, eax
    mov dword ptr [k_proc_next_thread], eax

    /* before we can start poping stuff from the next thread stack we need to set up its 
    page directory, since the stack is mapped only in it. */
    mov esp, dword ptr [ebx + 36]
    mov eax, cr3
    cmp eax, dword ptr [ebx + 40]
    jz _cr3_already_set
    /* make sure we don't set cr3 twice when switching to the scheduler thread. Wiping the 
    tlb clean once is already enough pain */
    mov eax, dword ptr [ebx + 40]
    mov cr3, eax
    _cr3_already_set:

    /* restore sregs */
    pop eax
    mov fs, ax
    pop eax
    mov es, ax
    pop eax
    mov ds, ax
    /* restore gprs */
    pop ebp
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    
    iret
    