.intel_syntax noprefix
.code32


.section .text

.global k_int_disable_interrupts
k_int_disable_interrupts:
    cli
    ret

.global k_int_enable_interrupts
k_int_enable_interrupts:
    sti
    ret

.global k_int_lidt
k_int_lidt:
    sub esp, 6
    mov ax, offset k_int_idt_end
    sub ax, offset k_int_idt + 1
    mov word ptr [esp], ax
    mov eax, offset k_int_idt
    mov dword ptr [esp + 2], eax
    lidt [esp]
    add esp, 6
    ret

.global k_int0_a
k_int0_a:
    call k_int_int0_handler
    hlt

.global k_int1_a
k_int1_a:
    hlt
    
.global k_int3_a
k_int3_a:
    call k_int_int3_handler
    iret

.global k_int4_a
k_int4_a:
    call k_int_int4_handler
    iret

.global k_int5_a
k_int5_a:
    call k_int_int5_handler
    iret

.global k_int6_a
k_int6_a:
    call k_int_int6_handler
    hlt
    iret

.global k_int7_a
k_int7_a:
    call k_int_int7_handler
    iret

.global k_int8_a
k_int8_a:
    call k_int_int8_handler
    hlt

.global k_int13_a
k_int13_a:
    call k_int_int13_handler
    hlt
    iret

.global k_int14_a
k_int14_a:
    sub esp, 4
    mov eax, cr2
    mov [esp], eax
    call k_int_int14_handler
    add esp, 4
    hlt
    iret

.global k_intn_a
k_intn_a:
    call k_int_intn_handler
    hlt

.section .data
.balign 8
.global k_int_idt
k_int_idt:
.rept 22
.quad 0
.endr
k_int_idt_end:
