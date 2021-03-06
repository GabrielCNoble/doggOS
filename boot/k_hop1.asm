.intel_syntax noprefix
.code16


.section .text  

_start:
    jmp _kern16_start

_kern16_start:
    mov ax, offset gdt_end - gdt_start - 1
    mov [gdt_size], ax
    mov eax, offset gdt_start
    mov dword ptr [gdt_offset], eax
    lgdt [gdt_ptr]

    mov ax, ds
    mov es, ax
    mov di, offset k_mem_ranges
    mov edx, 0x534d4150
    mov ebx, 0x00000000
    clc

    _mem_map_loop_start:
        mov eax, 0x0000e820
        mov ecx, 24
        int 0x15
        cmp ebx, 0
        /* if ebx is zero after the interrupt, it probably means we're done */
        jz _mem_map_loop_end
        /* however, some bioses signal the end of the list by setting the carry
        flag, so we test that too */
        jc _mem_map_loop_end
        
        /* test whether this range is available */
        cmp dword ptr es:[di + 16], 0x1
        jnz _mem_map_loop_start

        /* test whether this range has a non-zero size */
        cmp dword ptr es:[di + 8], 0x0
        jz _mem_map_loop_start
        /* range available */
        add di, 24
        inc dword ptr [k_mem_range_count]
        jmp _mem_map_loop_start

        _mem_map_loop_end:



    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    call 0x0010:_kern_start
 
.section .data
gdt_ptr:
gdt_size: .short 0
gdt_offset: .int 0

.align 4
gdt_start:
/* throw-away gdt, only for the sake of entering protected mode */
null_segment: .short 0, 0, 0, 0
data_segment: .short 0xffff, 0x0000, 0x8000|0x1000|0x0200, 0x0040|0x000f
code_segment: .short 0xffff, 0x0000, 0x8000|0x1000|0x0a00, 0x0040|0x000f
gdt_end:

.align 4
.global k_mem_range_count
k_mem_range_count: .int 0

.align 8
.global k_mem_ranges
k_mem_ranges:
mem_map_start:
.rept 32
.quad 0 /* range base address */
.quad 0 /* range length */
.quad 0 /* range type */
.endr
mem_map_end:

