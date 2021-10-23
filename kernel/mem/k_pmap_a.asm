.intel_syntax noprefix
.code32

.section .text

.global k_mem_GetPageMap
k_mem_GetPageMap:
    mov eax, cr3
    ret

/* memory ranges */
.balign 8
.global k_mem_low_range
k_mem_low_range:
.quad 0
.quad 0
.quad 0
