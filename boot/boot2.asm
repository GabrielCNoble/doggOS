.intel_syntax noprefix
.code16

.section .text

stage2_start:
    mov dword ptr [origin_drive], edx
    /* query the memory map of the machine */
    mov ax, ds
    mov es, ax
    mov di, offset mem_ranges
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
        cmp dword ptr es:[edi + 16], 1
        jnz _mem_map_loop_start

        /* test whether this range has a non-zero size */
        cmp dword ptr es:[edi + 8], 0
        jz _mem_map_loop_start
        /* range available */

        /* test whether this range is the lower memory */
        cmp dword ptr es:[edi], 0
        /* low memory, so ignore */
        jz _mem_map_loop_start
        /* is low memory, so to simplify things in the kernel code we'll put it in a separate variable */

    _append_range:
        add di, 24
        inc dword ptr [range_count]
        jmp _mem_map_loop_start

        _mem_map_loop_end:

    /* read the boot info */
    mov si, offset read_packet
    mov edx, dword ptr [origin_drive]
    /* 1 sector */
    mov ax, 1
    mov word ptr [si + 2], ax
    /* we'll use the same load address of the kernel to store the boot info */
    mov ax, 0x7c00
    mov word ptr [si + 4], ax
    mov ax, 0
    mov word ptr [si + 6], ax
    /* boot info is in sector 8 */
    mov eax, 4
    mov dword ptr [si + 8], eax
    mov ax, 0x4200
    int 0x13


    xor dx, dx
    mov si, offset read_packet


    /* mov ax, offset k_kernel_end
    sub ax, offset k_kernel_start */
    mov ecx, 0x7c00
    mov eax, dword ptr [ecx + 4]
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
    /* offset */
    mov ax, 0x7c00
    mov word ptr [si + 4], ax
    /* segment */
    mov ax, 0
    mov word ptr [si + 6], ax
    /* mov eax, offset k_kernel_sector */
    mov eax, dword ptr [ecx]
    mov dword ptr [si + 8], eax
    /* restore the drive info */
    mov edx, dword ptr[origin_drive]
    mov ax, 0x4200
    /* do the thing */
    int 0x13

    /* load gdt */
    mov ax, offset gdt_end
    sub ax, offset gdt_start + 1
    mov [gdt_size], ax
    mov eax, offset gdt_start
    mov dword ptr [gdt_offset], eax
    lgdt [gdt_ptr]

    /* enable protected mode */
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x0010:set_cs

.code32
    set_cs:
    mov ax, 0x8
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    /* push the kernel stack to the end of the usable low memory */
    mov esp, 0x7fff0

    /* initialize the init data */
    mov eax, offset mem_ranges
    mov dword ptr [ranges], eax
    mov eax, dword ptr [origin_drive]
    mov dword ptr [boot_drive], eax

    /* stdcall */
    sub esp, 4
    mov eax, offset init_data
    mov dword ptr [esp], eax
    /* finally, start initializing kernel stuff */
    jmp 0x7c00
    hlt

.balign 4
init_data:
ranges:             .int 0
range_count:        .int 0
boot_drive:         .int 0

.balign 8
gdt_start:
null_segment: .short 0, 0, 0, 0
data_segment0: .short 0xffff, 0x0000, 0x8000|0x1000|0x0200, 0x00c0|0x000f
code_segment0: .short 0xffff, 0x0000, 0x8000|0x1000|0x0a00, 0x00c0|0x000f

data_segment3: .short 0xffff, 0x0000, 0x8000|0x7000|0x0200, 0x00c0|0x000f
code_segment3: .short 0xffff, 0x0000, 0x8000|0x7000|0x0a00, 0x00c0|0x000f
gdt_end:

.balign 4
gdt_ptr:
gdt_size: .short 0
gdt_offset: .int 0

.global origin_drive
origin_drive: .int 0

read_packet:
size:       .byte 0x10
res:        .byte 0x0
count:      .short 0x0002
offset:     .short 0x0500
segment:    .short 0x0000
lba:        .quad 0x00000001

.balign 8
mem_ranges:
.rept 32
.quad 0 /* range base address */
.quad 0 /* range length */
.quad 0 /* range type */
.endr

