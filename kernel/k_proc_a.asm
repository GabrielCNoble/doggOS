.intel_syntax noprefix
.code32

.section .text

.global k_proc_RunThread
k_proc_RunThread:
    mov eax, dword ptr [esp + 4]
    mov dword ptr [k_next_thread], eax
    int 38
    ret

.global k_proc_SwitchToThread
k_proc_SwitchToThread:
    /* save eax and eflags, since we'll need them ahead */
    push eax
    /* save all segment regs on the stack, to simplify things downstream */
    mov ax, gs
    push eax
    mov ax, fs
    push eax
    mov ax, es
    push eax
    mov ax, ds
    push eax
    mov ax, ss
    push eax

    /* we need to find out from which ring we just came from. If src ring != 0 it means
    a stack switch happened, and we have some extra stuff in the stack that we need to deal 
    with. Also, we'll need to load ring 0 data segment so we can access some globals */
    mov eax, dword ptr [esp + 36]
    and eax, 0x3
    cmp eax, 0
    jnz _already_ring0_ds
        mov ax, 0x0008
        mov ds, ax
    _already_ring0_ds:
    
    mov eax, dword ptr [k_active_thread]
    add eax, 8

    /* save most gp regs */
    mov dword ptr [eax + 44], ecx
    mov dword ptr [eax + 48], edx
    mov dword ptr [eax + 52], ebx
    mov dword ptr [eax + 60], ebp
    mov dword ptr [eax + 64], esi
    mov dword ptr [eax + 68], edi
    /* most gp regs are saved, so we can use them freely */
    mov ebx, eax
    /* save ss */
    pop eax
    mov word ptr [ebx + 80], ax
    /* save ds */
    pop eax
    mov word ptr [ebx + 84], ax
    /* save es */
    pop eax
    mov word ptr [ebx + 72], ax
    /* save fs */
    pop eax
    mov word ptr [ebx + 88], ax
    /* save gs */
    pop eax
    mov word ptr [ebx + 92], ax
    /* save eax */
    pop eax
    mov dword ptr [ebx + 40], eax
    /* save eip */
    mov eax, dword ptr [esp]
    mov dword ptr [ebx + 32], eax
    /* save cs */
    mov eax, dword ptr [esp + 4]
    mov word ptr [ebx + 76], ax
    /* save eflags */
    mov eax, dword ptr [esp + 8]
    mov dword ptr [ebx + 36], eax
    /* save esp */
    mov dword ptr [ebx + 56], esp

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
    add eax, 8

    /* restore es */
    mov bx, word ptr [eax + 72]
    mov es, bx
    /* restore ss */
    mov bx, word ptr [eax + 80]
    mov ss, bx
    /* restore ds */
    mov bx, word ptr [eax + 84]
    mov ds, bx
    /* restore fs */
    mov bx, word ptr [eax + 88]
    mov fs, bx
    /* restore gs */
    mov bx, word ptr [eax + 92]
    mov gs, bx

    /* restore thread stack */
    mov ebp, dword ptr [eax + 60]
    mov esp, dword ptr [eax + 56]
    
    /* restore eip */
    mov ebx, dword ptr [eax + 32]
    mov dword ptr [esp], ebx
    /* restore code segment selector */
    mov bx, word ptr [eax + 76]
    movzx ebx, bx
    mov dword ptr [esp + 4], ebx
    /* restore eflags */
    mov ebx, dword ptr [eax + 36]
    mov dword ptr [esp + 8], ebx
    
    /* restore thread gp regs */
    mov ebx, dword ptr [eax + 52]
    mov ecx, dword ptr [eax + 44]
    mov edx, dword ptr [eax + 48]
    mov edi, dword ptr [eax + 68]
    mov esi, dword ptr [eax + 64]
    mov eax, dword ptr [eax + 40]

    iret
    