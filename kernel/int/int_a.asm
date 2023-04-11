.intel_syntax noprefix
.code32


.section .text

.global k_int_Lidt
k_int_Lidt:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    mov eax, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebp + 12]
    sub esp, 6
    shl ebx, 3
    dec ebx
    mov word ptr [esp], bx
    mov dword ptr [esp + 2], eax
    lidt [esp]
    add esp, 6
    pop ebx
    pop eax
    pop ebp
    ret

