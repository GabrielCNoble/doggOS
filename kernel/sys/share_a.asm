.intel_syntax noprefix
.code32

.section .text

.global k_sys_SysCall
k_sys_SysCall:
    push ebp
    mov ebp, esp
    /* cli
    hlt */
    nop
    nop
    /* save registers that will be clobbered */
    push ebx
    push ecx
    push edx

    /* syscall number */
    mov eax, dword ptr [ebp + 8]
    /* other args */
    mov ebx, dword ptr [ebp + 12]
    mov ecx, dword ptr [ebp + 16]
    mov edx, dword ptr [ebp + 20]

    /* syscall ( ͡° ͜ʖ ͡°). Status in eax */
    int 69

    /* restore clobbered registers */
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret
