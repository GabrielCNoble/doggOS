.intel_syntax noprefix
.code32

.section .text

.global k_mem_load_pstate
k_mem_load_pstate:
    mov eax, dword ptr [esp + 4]
    mov eax, dword ptr [eax]
    mov cr3, eax
    ret

.global k_mem_paging_enabled
k_mem_paging_enabled:
    mov eax, cr0
    and eax, 0x80000000
    ret

.global k_mem_enable_paging_a
k_mem_enable_paging_a:
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

.global k_mem_disable_paging_a
k_mem_disable_paging_a:
    mov eax, cr0
    and eax, 0x7fffffff
    mov cr0, eax
    ret

.global k_mem_invalidate_tlb
k_mem_invalidate_tlb:
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
r1_data_segment: .short 0xffff, 0x0000, 0xa000|0x1000|0x0200, 0x00c0|0x000f
r1_code_segment: .short 0xffff, 0x0000, 0xa000|0x1000|0x0a00, 0x00c0|0x000f
r2_data_segment: .short 0xffff, 0x0000, 0xc000|0x1000|0x0200, 0x00c0|0x000f
r2_code_segment: .short 0xffff, 0x0000, 0xc000|0x1000|0x0a00, 0x00c0|0x000f
r3_data_segment: .short 0xffff, 0x0000, 0xe000|0x1000|0x0200, 0x00c0|0x000f
r3_code_segment: .short 0xffff, 0x0000, 0xe000|0x1000|0x0a00, 0x00c0|0x000f
.global k_mem_gdt_end
k_mem_gdt_end:

.global k_mem_gdt_desc_count
k_mem_gdt_desc_count: .int 10

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

/* page directories */
.balign 4096
.global k_mem_pdirs
k_mem_pdirs:
.rept 1024
.int 0
.endr 
