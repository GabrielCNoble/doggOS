.code32
.intel_syntax noprefix

.section .text

.global k_int0_a
k_int0_a:
    call k_int_Int0
    hlt

.global k_int2_a
k_int2_a:
    call k_int_Int2
    iret

.global k_int1_a
k_int1_a:
    hlt
    
.global k_int3_a
k_int3_a:
    call k_int_Int3
    hlt

.global k_int4_a
k_int4_a:
    call k_int_Int4
    iret

.global k_int5_a
k_int5_a:
    call k_int_Int5
    iret

.global k_int6_a
k_int6_a:
    call k_int_Int6
    hlt

.global k_int7_a
k_int7_a:
    call k_int_Int7
    iret

.global k_int8_temp_a
k_int8_temp_a:
    call k_int_Int8
    iret

.global k_int8_a
k_int8_a:
    call k_int_Int8
    hlt

.global k_int10_a
k_int10_a:
    call k_int_Int10
    hlt

.global k_int13_a
k_int13_a:
    call k_int_Int13
    hlt

.global k_int14_a
k_int14_a:
    cmp esp, 0x2000
    jne _skip
        hlt
    _skip:
    mov eax, cr2
    push eax
    call k_int_Int14
    pop eax
    hlt
    iret


.macro k_irq_jmp_table_entry from to
    .balign 16
    call k_proc_ExitThreadContext
    mov eax, \from
    jmp k_irq_DispatchIRQ_a
    .if \to-\from
        k_irq_jmp_table_entry "(\from+1)",\to
    .endif
.endm

.balign 16
.global k_irq_IrqJumpTable_a
k_irq_IrqJumpTable_a:
    k_irq_jmp_table_entry 32,64
    k_irq_jmp_table_entry 65,96
    k_irq_jmp_table_entry 97,128
    k_irq_jmp_table_entry 129,160
    k_irq_jmp_table_entry 161,192
    k_irq_jmp_table_entry 193,224
    k_irq_jmp_table_entry 225,256

.global k_irq_DispatchIRQ_a
k_irq_DispatchIRQ_a:
    push eax
    call k_irq_DispatchIRQ
    pop eax
    call k_proc_EnterThreadContext
    iret
