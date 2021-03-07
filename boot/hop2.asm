.intel_syntax noprefix
.section .text

.global _kern_start
_kern_start:
    mov ax, 0x8
    mov ds, ax
    mov ss, ax
    call k_main
    cli
    hlt
