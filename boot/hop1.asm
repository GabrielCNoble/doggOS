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

    /* we'll need dx here for a division, so we preserve it,
    since it contains the number of the driver we got loaded from */
    mov cx, dx
    xor dx, dx
    mov si, offset h0_read_packet
    mov ax, offset k_kernel_end
    sub ax, offset k_kernel_start
    mov bx, 0x200
    /* how many sectors the kernel currently takes */
    div bx
    cmp dx, 0
    jz _exact_div
    /* the kernel doesn't fit perfectly in a round number 
    of sectors, so we increment the number of sectors here
    to accomodate the remainder */
    inc ax
_exact_div:
    /* number of sectors */
    mov word ptr [si + 2], ax
    /* offset of where to drop the data. Segment remains the same */
    mov ax, 0x0500
    mov word ptr [si + 4], ax
    /* we'll start reading from the fourth sector */
    mov eax, 0x00000003
    mov dword ptr [si + 8], eax
    /* restore the drive info */
    mov dx, cx
    mov ax, 0x4200
    /* do the thing */
    int 0x13


    /* query the memory map of the machine */
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
    /* set protected mode bit */
    or eax, 0x1
    /* activate protected mode */
    mov cr0, eax
    /* reload code segment descriptor */
    call 0x0010:_set_cs

_set_cs:

.code32
    mov ax, 0x8
    mov ss, ax
    mov ds, ax
    call k_main
    cli
    hlt
 
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

