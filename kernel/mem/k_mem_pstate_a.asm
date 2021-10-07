.intel_syntax noprefix
.code32

.section .text

.global k_mem_LoadPageDir
k_mem_LoadPageDir:
    mov eax, dword ptr [esp + 4]
    mov cr3, eax
    ret

.global k_mem_GetPState
k_mem_GetPState:
    mov eax, cr3
    ret

/* memory ranges */
.balign 8
.global k_mem_low_range
k_mem_low_range:
.quad 0
.quad 0
.quad 0
