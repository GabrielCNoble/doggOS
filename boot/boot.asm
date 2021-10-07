.intel_syntax noprefix
.code16

.section .text 
.global _start 

_start:
    jmp stage0_start

stage0_start:
    
    cli
    cld

    /* store the drive we're coming from */
    mov dword ptr[origin_drive], edx
    mov ah, 0x42
    mov si, offset read_packet
    int 0x13
    jmp stage1_start

.balign 4

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

.global read_packet
read_packet:
size:       .byte 0x10
res:        .byte 0x0
count:      .short 0x0002
offset:     .short stage1_start
segment:    .short 0x0000
lba:        .quad 0x00000001

/* pad until the last two bytes in this 512 bytes block. Those bytes are the boot sector signature bytes */
.fill 0x1fe - (. - _start)
.short 0xaa55

stage1_start:
    jmp stage2_start

stage2_start:
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
    /*    mov esi, 0 */
    /* _copy_low_range: */
        /* we'll be moving 3 qwords here... */
        /* mov ecx, dword ptr es:[edi + esi * 4] */
        /* mov dword ptr [k_mem_low_range + esi * 4], ecx */
        /* cmp esi, 5 */
        /* jz _mem_map_loop_start */
        /* inc esi */
        /* jmp _copy_low_range */   
        

    _append_range:
        add di, 24
        inc dword ptr [range_count]
        jmp _mem_map_loop_start

        _mem_map_loop_end:

    xor dx, dx
    mov si, offset read_packet
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
    /* offset */
    mov ax, offset k_kernel_start
    mov word ptr [si + 4], ax
    /* segment */
    mov ax, 0
    mov word ptr [si + 6], ax
    /* we'll start reading from the fifth sector */
    mov eax, offset k_kernel_sector
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
    mov esp, 0x7ffff

    /* initialize the init data */
    mov eax, offset mem_ranges
    mov dword ptr [ranges], eax

    /* stdcall */
    sub esp, 4
    mov eax, offset init_data
    mov dword ptr [esp], eax
    /* finally, start initializing kernel stuff */
    call k_Init
    hlt

.balign 4
init_data:
ranges:             .int 0
range_count:        .int 0

.balign 8
mem_ranges:
.rept 32
.quad 0 /* range base address */
.quad 0 /* range length */
.quad 0 /* range type */
.endr
