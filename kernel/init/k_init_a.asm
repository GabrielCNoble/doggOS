.intel_syntax noprefix
.code32

.section .text

.global _start

_start:
k_Init_a:
    call k_Init
    hlt
