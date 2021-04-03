.intel_syntax noprefix
.code16
.section .text  

.global hop1_start
hop1_start:
    /* push the kernel stack waaaay forward, so the following function
    call won't potentially trash any of the data structures defined here */
    mov esp, 0x7ffff

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
    /* offset */
    mov ax, 0x0500
    mov word ptr [si + 4], ax
    /* segment */
    mov ax, 0
    mov word ptr [si + 6], ax
    /* we'll start reading from the fifth sector */
    mov eax, offset k_kernel_sector
    mov dword ptr [si + 8], eax
    /* restore the drive info */
    mov dx, cx
    mov ax, 0x4200
    /* do the thing */
    int 0x13

    /* load the gdt */
    mov ax, offset k_mem_gdt_end
    sub ax, offset k_mem_gdt + 1
    mov [gdt_size], ax
    mov eax, offset k_mem_gdt
    mov dword ptr [gdt_offset], eax
    lgdt [gdt_ptr]

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
        cmp dword ptr es:[edi + 16], 1
        jnz _mem_map_loop_start

        /* test whether this range has a non-zero size */
        cmp dword ptr es:[edi + 8], 0
        jz _mem_map_loop_start
        /* range available */

        /* test whether this range is the lower memory */
        cmp dword ptr es:[edi], 0
        /* not low memory */
        jnz _append_range
        /* is low memory, so to simplify things in the kernel code we'll put it in a separate variable */
        mov esi, 0
    _copy_low_range:
        /* we'll be moving 3 qwords here... */
        mov ecx, dword ptr es:[edi + esi * 4]
        mov dword ptr [k_mem_low_range + esi * 4], ecx
        cmp esi, 5
        jz _mem_map_loop_start
        inc esi
        jmp _copy_low_range
        

    _append_range:
        add di, 24
        inc dword ptr [k_mem_range_count]
        jmp _mem_map_loop_start

        _mem_map_loop_end:

    /* enable protected mode */
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    /* finally, start initializing kernel stuff */
    call 0x0010:k_init_a
 
.section .data
.align 4
gdt_ptr:
gdt_size: .short 0
gdt_offset: .int 0
