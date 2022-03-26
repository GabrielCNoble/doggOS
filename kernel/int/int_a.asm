.intel_syntax noprefix
.code32


.section .text

.global k_int_Lidt
k_int_Lidt:
    mov eax, dword ptr [esp + 4]
    mov ebx, dword ptr [esp + 8]
    sub esp, 6
    shl ebx, 3
    dec ebx
    mov word ptr [esp], bx
    mov dword ptr [esp + 2], eax
    lidt [esp]
    add esp, 6
    ret

