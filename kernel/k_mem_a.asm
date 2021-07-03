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

.global k_mem_IsPagingEnabled
k_mem_IsPagingEnabled:
    mov eax, cr0
    and eax, 0x80000000
    ret

.global k_mem_EnablePaging
k_mem_EnablePaging:
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

.global k_mem_DisablePaging
k_mem_DisablePaging:
    mov eax, cr0
    and eax, 0x7fffffff
    mov cr0, eax
    ret

.global k_mem_InvalidateTLB
k_mem_InvalidateTLB:
    mov eax, dword ptr [esp + 4]
    invlpg dword ptr [eax]
    ret

.section .data
/* gdt */
.balign 8
.global k_mem_gdt
k_mem_gdt:
null_segment: .short 0, 0, 0, 0
r0_data_segment: .short 0xffff, 0x0000, 0x8000|0x1000|0x0200, 0x00c0|0x000f
r0_code_segment: .short 0xffff, 0x0000, 0x8000|0x1000|0x0a00, 0x00c0|0x000f
.global k_mem_gdt_end
k_mem_gdt_end:

.global k_mem_gdt_desc_count
k_mem_gdt_desc_count: .int 3

/* memory ranges */
.balign 8
.global k_mem_low_range
k_mem_low_range:
.quad 0
.quad 0
.quad 0

.global k_mem_ranges
k_mem_ranges:
.rept 32
.quad 0 /* range base address */
.quad 0 /* range length */
.quad 0 /* range type */
.endr

.global k_mem_range_count
k_mem_range_count: .int 0
