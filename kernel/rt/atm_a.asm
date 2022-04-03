.code32
.intel_syntax noprefix
.section .text

.global k_rt_Xchg32
k_rt_Xchg32:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    mov ebx, dword ptr [ebp + 8]        /* location */
    mov ecx, dword ptr [ebp + 12]        /* new value */
    xchg dword ptr [ebx], ecx
    mov ebx, dword ptr [ebp + 16]       /* old value */
    mov dword ptr [ebx], ecx
    pop ecx
    pop ebx
    pop ebp
    ret

.global k_rt_CmpXchg
k_rt_CmpXchg:
.global k_rt_CmpXchg32
k_rt_CmpXchg32:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    mov ebx, dword ptr [ebp + 8]        /* location */
    mov ecx, dword ptr [ebp + 16]       /* new value */
    mov eax, dword ptr [ebp + 12]        /* cmp value */
    lock cmpxchg dword ptr [ebx], ecx
    mov ebx, dword ptr [ebp + 20]       /* old value */
    mov ecx, eax
    setz al
    movzx eax, al
    cmp ebx, 0
    je _skip_old_value
    mov dword ptr [ebx], ecx
    _skip_old_value:
    pop ecx
    pop ebx
    pop ebp
    ret

.global k_rt_Inc32Wrap
k_rt_Inc32Wrap:
    mov ebx, dword ptr [esp + 4]
    mov eax, 1
    lock xadd dword ptr [ebx], eax
    ret

.global k_rt_Inc32Clamp
k_rt_Inc32Clamp:
    mov ebx, dword ptr [esp + 4]
    mov ecx, dword ptr [esp + 8]
    mov eax, dword ptr [ebx]
    xor ecx, eax
    jz _inc32_clamp_max_value_reached
    lea ecx, dword ptr [eax + 1]
    lock cmpxchg dword ptr [ebx], ecx
    setz cl
    movzx ecx, cl
    _inc32_clamp_max_value_reached:
    mov ebx, dword ptr [esp + 12]
    mov dword ptr [ebx], eax
    mov eax, ecx
    ret

.global k_rt_Dec32Wrap
k_rt_Dec32Wrap:
    mov ebx, dword ptr [esp + 4]
    mov eax, 0xffffffff
    lock xadd dword ptr [ebx], eax
    ret

.global k_rt_Dec32Clamp
k_rt_Dec32Clamp:
    mov ebx, dword ptr [esp + 4]
    mov ecx, dword ptr [esp + 8]
    mov eax, dword ptr [ebx]
    xor ecx, eax
    jz _dec32_clamp_max_value_reached
    lea ecx, dword ptr [eax - 1]
    lock cmpxchg dword ptr [ebx], ecx
    setz cl
    movzx ecx, cl
    _dec32_clamp_max_value_reached:
    mov ebx, dword ptr [esp + 12]
    mov dword ptr [ebx], eax
    mov eax, ecx
    ret

.global k_rt_SpinLock
k_rt_SpinLock:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    mov ecx, dword ptr [ebp + 8]        /* spinlock */
    mov eax, dword ptr [ecx]
    _spinlock_loop:
        and eax, 0xfffffffe
        lea ebx, [eax + 1]
        lock cmpxchg dword ptr [ecx], ebx
        jnz _spinlock_loop
    _spinlock_loop_exit:
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

.global k_rt_TrySpinLock
k_rt_TrySpinLock:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    mov ecx, dword ptr [ebp + 8]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    lea ebx, [eax + 1]
    lock cmpxchg dword ptr [ecx], ebx
    setz al
    movzx eax, al
    pop ecx
    pop ebx
    pop ebp
    ret

.global k_rt_SpinUnlock
k_rt_SpinUnlock:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    mov ecx, dword ptr [ebp + 8]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    xchg dword ptr [ecx], eax
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret
    
    
.global k_rt_SignalCondition
k_rt_SignalCondition:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    mov eax, 1
    mov ebx, dword ptr [ebp + 8]
    xchg dword ptr [ebx], eax
    pop ebx
    pop eax
    pop ebp
    ret
    
  .global k_rt_ClearCondition
  k_rt_ClearCondition:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    mov eax, 0
    mov ebx, dword ptr [ebp + 8]
    xchg dword ptr [ebx], eax
    pop ebx
    pop eax
    pop ebp
    ret
