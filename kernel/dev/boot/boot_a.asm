.code32
.intel_syntax noprefix
.section .text

.global k_BOOT_Read_a
k_BOOT_Read_a:
    push ebp
    mov ebp, esp
    push esi
    push ebx
    push ecx
    push edx

    mov eax, cr0
    push eax
    mov eax, cr3
    push eax

    /* save current idt */
    sub esp, 8
    sidt [esp]

    /* disable paging and flush tlb */
    mov ebx, cr0
    and ebx, 0x7fffffff
    mov cr0, ebx
    mov ebx, 0
    mov cr3, ebx

    /* disk transfer packet */
    sub esp, 16
    mov esi, esp

    mov eax, 0x10
    mov word ptr[esi], ax
    mov eax, dword ptr[ebp + 16]
    /* block count */
    mov word ptr[esi + 2], ax
    /* buffer */
    mov eax, dword ptr [ebp + 20]

    /* format buffer address into segment:offset format */
    mov word ptr[esi + 4], ax
    and eax, 0x00ff0000
    shr eax, 4
    mov word ptr[esi + 6], ax
    mov eax, dword ptr[ebp + 8]
    mov dword ptr [esi + 8], eax
    mov eax, dword ptr[ebp + 12]
    mov dword ptr [esi + 12], eax

    /* format packet address into segment:offset format */
    mov eax, esi
    and eax, 0x00ff0000
    shr eax, 4

    jmp 0x0028:k_BOOT_Read_a16_1
.code16
k_BOOT_Read_a16_1:
    /* exit protected mode */
    mov ebx, cr0
    and ebx, 0xfffffffe
    mov cr0, ebx
    jmp 0x0000:k_BOOT_Read_a16_2
k_BOOT_Read_a16_2:

    mov ebx, esp
    mov cx, ss
    mov dx, ds
    mov ds, ax
    mov ax, 0
    mov es, ax
    mov eax, ebx
    and eax, 0x00ff0000
    shr eax, 4
    mov ss, ax
    and esp, 0x0000ffff
    push ebx
    push ecx
    push edx
    push ebp
    mov ebp, esp

    /* load real mode ivt. Ideally the value set by the bios would be stored
    somewhere and used here, but for now this will do */
    sub esp, 6
    mov ebx, 0x3ff
    mov word ptr [esp], bx
    mov ebx, 0
    mov dword ptr [esp + 2], ebx
    lidt [esp]
    add esp, 6

    mov dx, 0x0080
    mov ax, 0x4200
    int 0x13

    pop ebp
    pop edx
    pop ecx
    pop ebx
    mov esp, ebx

    /* enter protected mode */
    mov ebx, cr0
    or ebx, 0x00000001
    mov cr0, ebx

    mov ss, cx
    mov ds, dx
    mov es, dx

    jmp 0x10:k_BOOT_Read_a32
.code32
k_BOOT_Read_a32:
    /* deallocate packet */
    add esp, 16
    
    /* restore previous idt */
    lidt [esp]
    add esp, 8

    /* restore paging structures (if any) */
    pop ebx
    mov cr3, ebx

    /* reenable paging (if previously enabled) */
    pop ebx
    mov ecx, cr0
    or ecx, ebx
    mov cr0, ecx
    pop edx
    pop ecx
    pop ebx
    pop esi
    pop ebp
    ret
