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
    /* save eax, since we'll need it ahead */
    push eax
    /* push ds, since we may need to modify it in case there was a stack switch */
    mov ax, ds
    push eax
    /* push ss, since we may need to modify it in case there was a stack switch */
    mov ax, ss
    push eax
    /* adjust esp value so it points to the return address. This is for the case a 
    stack switch hasn't happened, in which case the value stored here won't be overwritten */
    lea eax, dword ptr [esp + 12]
    push eax

    /* we need to find out from which ring we just came from. If src ring != 0 it means
    a stack switch happened, and we have some extra stuff in the stack that we need to deal 
    with. Also, we'll need to load ring 0 data segment so we can access some globals */
    mov eax, dword ptr [esp + 20]
    and eax, 0x3
    cmp eax, 0
    jz _already_in_ring0
        /* a stack switch happened, which means ss and esp from the currently running
        thread got pushed before eflags. To simplify things further down, we push ss 
        and esp to reserve enough space on the stack, and then move the values pushed
        by the cpu there. The code that will save those pointers then remain the same
        whether there's a stack switch or not */

        /* copy ss */
        mov eax, dword ptr [esp + 28]
        mov word ptr [esp + 4], ax
        /* copy esp */
        mov eax, dword ptr [esp + 24]
        mov dword ptr [esp], eax
        /* load ring 0 data segment selector so we can access some globals */
        mov ax, 0x0008
        mov ds, ax
    _already_in_ring0:
    
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
    /* save esp */
    pop eax
    mov dword ptr [ebx + 56], eax
    /* save ss */
    pop eax
    mov word ptr [ebx + 80], ax
    /* save ds */
    pop eax
    mov word ptr [ebx + 84], ax
    /* save eax */
    pop eax
    mov dword ptr [ebx + 40], eax
    /* save es */
    mov ax, es
    mov word ptr [ebx + 72], ax
    /* save fs */
    mov ax, fs
    mov word ptr [ebx + 88], ax
    /* save gs */
    /* mov ax, gs
    mov word ptr [ebx + 92], ax */
    /* save eip */
    mov eax, dword ptr [esp]
    mov dword ptr [ebx + 32], eax
    /* save cs */
    mov eax, dword ptr [esp + 4]
    mov word ptr [ebx + 76], ax
    /* save eflags */
    mov eax, dword ptr [esp + 8]
    mov dword ptr [ebx + 36], eax
    /* save cr3 */
    mov eax, cr3
    mov dword ptr [ebx + 28], eax

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
    /* restore fs */
    mov bx, word ptr [eax + 88]
    mov fs, bx
    /* we'll sacrifice gs here to allow initializing ds */
    mov bx, ds
    mov gs, bx
    /* restore ds */
    mov bx, word ptr gs:[eax + 84]
    mov ds, bx

    /* test whether we're going to return to another ring. If that's the case then the processor will switch
    to the thread stack for us, but we'll also need to push some extra things in the current stack */
    xor ecx, ecx
    mov cx, word ptr gs:[eax + 76]
    and ecx, 0x3
    cmp ecx, 0
    jz _ring0_thread
        /* we'll be returning to a different privilege level. The cpu expects to find in the stack:

        esp + 16: ss
        esp + 12: esp
        esp + 8:  eflags
        esp + 4:  cs
        esp + 0:  eip  

        to put the stack in the correct state we need to "shift" eip, cs and eflags down by two
        4 byte slots. The code further down already deals with putting them on the stack, so all
        we have to do here is copy esp and ss and subtract 8 from the stack */
        mov ecx, dword ptr gs:[k_proc_tss]
        xor edx, edx
        mov dx, word ptr [ecx + 8]
        mov ss, dx
        mov edx, dword ptr [ecx + 4]
        mov esp, edx

        /* copy esp */
        mov ecx, dword ptr gs:[eax + 56]
        mov dword ptr [esp + 12], ecx
        /* copy ss */
        xor ecx, ecx
        mov cx, word ptr gs:[eax + 80]
        mov dword ptr [esp + 16], ecx
        /* now adjust the stack so esp points at eip again */
        
        jmp _stack_adjustment_handled
    _ring0_thread:
        /* restore ss */
        mov bx, word ptr gs:[eax + 80]
        mov ss, bx
        /* restore esp */
        mov esp, dword ptr gs:[eax + 56]
    _stack_adjustment_handled:

    /* restore ebp */
    mov ebp, dword ptr gs:[eax + 60]
    /* restore eip */
    mov ebx, dword ptr gs:[eax + 32]
    mov dword ptr [esp], ebx
    /* restore code segment selector */
    xor ebx, ebx
    mov ebx, dword ptr gs:[eax + 76]
    mov dword ptr [esp + 4], ebx
    /* restore eflags */
    mov ebx, dword ptr gs:[eax + 36]
    mov dword ptr [esp + 8], ebx
    /* restore cr3 */
    mov ebx, dword ptr gs:[eax + 28]
    mov cr3, ebx


    /* restore thread gp regs */
    mov ebx, dword ptr gs:[eax + 52]
    mov ecx, dword ptr gs:[eax + 44]
    mov edx, dword ptr gs:[eax + 48]
    mov edi, dword ptr gs:[eax + 68]
    mov esi, dword ptr gs:[eax + 64]
    mov eax, dword ptr gs:[eax + 40]
    nop
    nop
    iret
    