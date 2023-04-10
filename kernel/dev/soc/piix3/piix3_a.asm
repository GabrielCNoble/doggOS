.intel_syntax noprefix
.code32


.section .text
.global k_PIIX3_IDE_Handler_a
k_PIIX3_IDE_Handler_a:
    call k_proc_ExitThreadContext
    sub esp, 4
    mov eax, k_PIIX3_IDE_disk
    mov dword ptr [esp], eax
    call k_PIIX3_IDE_Handler
    mov eax, 14
    mov dword ptr [esp], eax
    call k_PIIX3_ISA_EndOfInterrupt
    add esp, 4
    call k_proc_EnterThreadContext
    iret