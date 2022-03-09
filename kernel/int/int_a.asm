.intel_syntax noprefix
.code32


.section .text

.global k_int_Lidt
k_int_Lidt:
    mov eax, dword ptr [esp + 4]
    mov ebx, dword ptr [esp + 8]
    sub esp, 6
    shl ebx, 3
    dec ebx
    mov word ptr [esp], bx
    mov dword ptr [esp + 2], eax
    lidt [esp]
    add esp, 6
    ret

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
    mov eax, cr2
    push eax
    call k_int_Int14
    pop eax
    hlt
    iret

.global k_intn_a
k_intn_a:
    call k_int_Intn
    iret

.global k_int32_a
k_int32_a:
    pusha
    call k_int_Int32
    popa
    iret

.global k_int33_a
k_int33_a:
    pusha
    call k_int_Int33
    popa
    iret

.global k_int34_a
k_int34_a:
    pusha
    call k_int_Int34
    call k_apic_EndOfInterrupt
    popa
    iret

.global k_int35_a
k_int35_a:
    pusha
    call k_int_Int35
    call k_apic_EndOfInterrupt
    popa
    iret

.global k_int36_a
k_int36_a:
    pusha
    call k_int_Int36
    call k_apic_EndOfInterrupt
    popa
    iret

.global k_int69_a
k_int69_a:
    pusha
    call k_int_Int69
    call k_apic_EndOfInterrupt
    popa
    iret

