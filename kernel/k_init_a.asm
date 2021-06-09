.intel_syntax noprefix
.code32
.section .text


.global k_init_a
k_init_a:
    mov ax, 0x8
    mov ss, ax
    mov ds, ax
    mov es, ax
    call k_init
    hlt
