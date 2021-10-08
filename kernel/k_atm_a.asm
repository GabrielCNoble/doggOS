.code32
.intel_syntax noprefix

.section .text

.global k_atm_Xcgh32
k_atm_Xcgh32:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov eax, dword ptr [esp + 8]        /* new value */
    xchg dword ptr [ebx], eax
    ret

.global k_atm_Xcgh16
k_atm_Xcgh16:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ax, word ptr [esp + 8]          /* new value */
    xchg word ptr [ebx], ax
    movzx eax, ax
    ret

.global k_atm_Xcgh8
k_atm_Xcgh8:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov al, byte ptr [esp + 8]          /* new value */
    xchg byte ptr [ebx], al
    movzx eax, al
    ret

.global k_atm_CmpXcgh32
k_atm_CmpXcgh32:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ecx, dword ptr [esp + 8]        /* new value */
    mov eax, dword ptr [ebx]
    lock cmpxchg dword ptr [ebx], ecx
    jz _cmpxchg32_success
        mov ebx, dword ptr [esp + 12]   /* old value */
        mov dword ptr [ebx], eax
    _cmpxchg32_success:
    setz al
    movzx eax, al
    ret

.global k_atm_CmpXcgh16
k_atm_CmpXcgh16:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ecx, dword ptr [esp + 8]        /* new value */
    mov eax, dword ptr [ebx]
    lock cmpxchg dword ptr [ebx], ecx
    jz _cmpxchg16_success
        mov ebx, dword ptr [esp + 12]   /* old value */
        mov dword ptr [ebx], eax
    _cmpxchg16_success:
    setz al
    movzx eax, al
    ret

.global k_atm_CmpXcgh8
k_atm_CmpXcgh8:
    mov ebx, dword ptr [esp + 4]        /* location */
    mov ecx, dword ptr [esp + 8]        /* new value */
    mov eax, dword ptr [ebx]
    lock cmpxchg dword ptr [ebx], ecx
    jz _cmpxchg8_success
        mov ebx, dword ptr [esp + 12]   /* old value */
        mov dword ptr [ebx], eax
    _cmpxchg8_success:
    setz al
    movzx eax, al
    ret

.global k_atm_SpinLock
k_atm_SpinLock:
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    _spinlock_loop:
        and eax, 0xfffffffe
        lea ebx, [eax + 1]
        lock cmpxchg dword ptr [ecx], ebx
        jnz _spinlock_loop
    _spinlock_loop_exit:
    ret

.global k_atm_TrySpinLock
k_atm_TrySpinLock:
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    lea ebx, [eax + 1]
    lock cmpxchg dword ptr [ecx], ebx
    setz al
    movzx eax, al
    ret

.global k_atm_SpinUnlock
k_atm_SpinUnlock:
    mov ecx, dword ptr [esp + 4]        /* spinlock */
    mov eax, dword ptr [ecx]
    and eax, 0xfffffffe
    xchg dword ptr [ecx], eax
    ret
