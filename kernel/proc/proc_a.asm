.intel_syntax noprefix
.code32

.section .text
.include "proc/defs_a.inc"

/* Saves most of the thread registers in the r0 stack. This function preserves
the original values of the registers. */
.global k_proc_PushThreadRegs
k_proc_PushThreadRegs:
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

    /* restore return address */
    push ebx
    /* restore clobbered regs */
    mov eax, dword ptr [esp + 40]
    mov ebx, dword ptr [esp + 36]
    ret

/* Restores most of the thread registers from the current r0 stack. This function
blindly pops stuff out of the stack. */
.global k_proc_PopThreadRegs
k_proc_PopThreadRegs:
    /* save return address */
    pop eax

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

    /* store return address in a temp var */
    mov dword ptr [esp - 4], eax
    pop eax
    /* restore return address */
    push dword ptr [esp - 8]
    ret

    
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

/* Switch between the scheduler thread and a normal thread, or between a normal thread and
the scheduler thread */
.global k_proc_PreemptThread
k_proc_PreemptThread:
    nop
    nop
    nop
    nop
    nop
    nop
    call k_proc_PushThreadRegs
    /* we need to save cr3 in the thread struct, but chances are we're currently in user 
    space, so that's not accessible. Instead, we hold onto its value until we change to 
    the kernel address space */
    mov eax, cr3
    mov edx, offset k_proc_core_state
    cmp dword ptr [edx + k_proc_core_state_next_thread], 0
    jnz _valid_next_thread
        /* next thread is NULL, which means we're going back to the scheduler. For that to
        work, we'll need to load the kernel page dir before trying to access anything other
        than the core state */

        /* scheduler thread */
        lea ebx, dword ptr [edx + k_proc_core_state_scheduler_thread]
        mov dword ptr [edx + k_proc_core_state_next_thread], ebx

        mov ecx, cr3
        cmp ecx, k_proc_page_map
        je _valid_next_thread
            /* change to kernel address space */
            mov ecx, k_proc_page_map
            mov cr3, ecx

    _valid_next_thread:

    /* next thread */
    mov ebx, dword ptr [edx + k_proc_core_state_next_thread]
    /* current thread */
    mov ecx, dword ptr [edx + k_proc_core_state_current_thread]
    /* save page map of current thread */
    mov dword ptr [ecx + k_proc_thread_current_pmap], eax
    /* save esp of current thread */
    mov dword ptr [ecx + k_proc_thread_current_sp], esp
    /* swap current thread for next thread, and clear next thread pointer */
    mov dword ptr [edx + k_proc_core_state_current_thread], ebx
    mov dword ptr [edx + k_proc_core_state_next_thread], 0

    /* store the start of the switch stack segment into the loaded tss. This is just useful 
    for threads running out of ring 0 */
    mov ecx, dword ptr [ebx + k_proc_thread_start_sp]
    
    /* core tss */
    mov eax, dword ptr [edx + k_proc_core_state_tss]
    mov dword ptr [eax + 4], ecx

    /* before we can start poping stuff from the next thread stack we need to set up its 
    page directory, since the stack is mapped only in it. */
    mov esp, dword ptr [ebx + k_proc_thread_current_sp]
    mov eax, cr3
    cmp eax, dword ptr [ebx + k_proc_thread_current_pmap]
    je _cr3_already_set
        /* make sure we don't set cr3 twice when switching to the scheduler thread. Wiping the 
        tlb clean once is already enough pain */
        mov eax, dword ptr [ebx + k_proc_thread_current_pmap]
        mov cr3, eax
    _cr3_already_set:

    call k_proc_PopThreadRegs
    iret
    