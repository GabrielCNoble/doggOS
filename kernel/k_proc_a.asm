.intel_syntax noprefix
.code32

.section .text

.global k_proc_SwitchToThread
k_proc_SwitchToThread:
    mov eax, dword ptr [esp + 4]
    mov dword ptr [k_next_thread], eax
    int 38
    ret

.global k_proc_RunThread
k_proc_RunThread:
    /* push eax, since we'll need to use it here */
    push eax
    mov eax, dword ptr [k_active_thread]
    /* save gp regs */
    mov dword ptr [eax + 20], ebp
    mov dword ptr [eax + 28], ebx
    mov dword ptr [eax + 32], ecx
    mov dword ptr [eax + 36], edx
    mov dword ptr [eax + 40], edi
    mov dword ptr [eax + 44], esi
    /* we saved all other registers, so we can pop eax and use
    one of the other registers to continue saving stuff */
    mov ebx, eax
    pop eax
    /* save eax */
    mov dword ptr [ebx + 24], eax
    /* save esp */
    mov dword ptr [ebx + 16], esp
    /* save eip */
    mov eax, dword ptr [esp]
    mov dword ptr [ebx + 12], eax
    /* save eflags */
    mov eax, dword ptr [esp + 8]
    mov dword ptr [ebx + 48], eax
    /* save cs */
    mov eax, dword ptr [esp + 4]
    mov dword ptr [ebx + 52], eax

    /* now start setting stuff up for the next thread */
    mov ebx, dword ptr [k_next_thread]
    cmp ebx, 0
    jne _valid_next_thread
        /* we don't have a next thread pointer, which means we're going back to the scheduler */
        mov ebx, offset k_scheduler_thread
    _valid_next_thread:

    mov dword ptr [k_active_thread], ebx
    xor eax, eax
    mov dword ptr [k_next_thread], eax
    mov eax, ebx

    /* restore thread stack */
    mov ebp, dword ptr [eax + 20]
    mov esp, dword ptr [eax + 16]
    
    
    /* restore eip */
    mov ebx, dword ptr [eax + 12]
    mov dword ptr [esp], ebx
    /* restore code segment selector */
    mov ebx, dword ptr [eax + 52]
    mov dword ptr [esp + 4], ebx
    /* restore eflags */
    mov ebx, dword ptr [eax + 48]
    mov dword ptr [esp + 8], ebx

    /* restore thread gp regs */
    mov ebx, dword ptr [eax + 28]
    mov ecx, dword ptr [eax + 32]
    mov edx, dword ptr [eax + 36]
    mov edi, dword ptr [eax + 40]
    mov esi, dword ptr [eax + 44]
    mov eax, dword ptr [eax + 24]

    iret