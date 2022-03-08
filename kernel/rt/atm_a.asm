.code32
.intel_syntax noprefix
.section .text

.global k_rt_Xcgh32
k_rt_Xcgh32:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ecx, dword ptr [esp + 8]        /* new value */
    xchg dword ptr [ebx], ecx
    mov ebx, dword ptr [esp + 12]       /* old value */
    mov dword ptr [ebx], ecx
    ret

.global k_rt_CmpXcgh
k_rt_CmpXcgh:
.global k_rt_CmpXcgh32
k_rt_CmpXcgh32:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ecx, dword ptr [esp + 12]       /* new value */
    mov eax, dword ptr [esp + 8]        /* cmp value */
    lock cmpxchg dword ptr [ebx], ecx
    mov ebx, dword ptr [esp + 16]       /* old value */
    mov ecx, eax
    setz al
    movzx eax, al
    cmp ebx, 0
    je _skip_old_value
    mov dword ptr [ebx], ecx
    _skip_old_value:
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
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    _spinlock_loop:
        and eax, 0xfffffffe
        lea ebx, [eax + 1]
        lock cmpxchg dword ptr [ecx], ebx
        jnz _spinlock_loop
    _spinlock_loop_exit:
    ret

.global k_rt_TrySpinLock
k_rt_TrySpinLock:
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    lea ebx, [eax + 1]
    lock cmpxchg dword ptr [ecx], ebx
    setz al
    movzx eax, al
    ret

.global k_rt_SpinUnlock
k_rt_SpinUnlock:
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    xchg dword ptr [ecx], eax
    ret
