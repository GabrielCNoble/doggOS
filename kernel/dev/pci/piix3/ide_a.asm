.intel_syntax noprefix
.code32


.section .text
.global k_PIIX3_IDE_Handler_a
k_PIIX3_IDE_Handler_a:
    call k_proc_ExitThreadContext
    call k_PIIX3_IDE_Handler
    call k_proc_EnterThreadContext
    iret