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
/* Switch between the scheduler thread and a normal thread, or between a normal thread and
the scheduler thread */
.global k_proc_PreemptThread_a
k_proc_PreemptThread_a:
    call k_proc_ExitThreadContext

    mov edx, offset k_proc_core_state
    mov ecx, dword ptr [edx + k_proc_core_state_current_thread]
    mov dword ptr [ecx + k_proc_thread_current_sp], esp

    /* next thread */
    mov ebx, dword ptr [edx + k_proc_core_state_next_thread]
    cmp ebx, 0
    jne _valid_next_thread2
        /* next thread is null, so we switch to the scheduler thread */
        lea ebx, dword ptr [edx + k_proc_core_state_scheduler_thread]
    _valid_next_thread2:

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

    call k_proc_EnterThreadContext
    iret
