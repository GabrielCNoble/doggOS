.intel_syntax noprefix
.code32


.section .text
/* .global k_PIIX3_IDE_Handler_a
k_PIIX3_IDE_Handler_a:
    call k_proc_ExitThreadContext
    sub esp, 4
    mov eax, k_PIIX3_IDE_disk
    mov dword ptr [esp], eax
    call k_IDE_InterruptHandler
    mov eax, 14
    mov dword ptr [esp], eax
    call k_PIIX3_ISA_EndOfInterrupt
    add esp, 4
    call k_proc_EnterThreadContext
    iret
*/

/* .global k_PIIX3_ISA_Timer1_Handler_a
k_PIIX3_ISA_Timer1_Handler_a:
    call k_proc_ExitThreadContext
    call k_PIIX3_ISA_Timer1_Handler
    call k_proc_EnterThreadContext
    iret
*/

/* .global k_PIIX3_PS2_KeyboardHandler_a
k_PIIX3_PS2_KeyboardHandler_a:
    call k_proc_ExitThreadContext
    call k_dev_kb_KeyboardHandler
    call k_proc_EnterThreadContext
    iret
*/

.global k_PIIX3_PS2_MouseHandler_a
k_PIIX3_PS2_MouseHandler_a:
    call k_proc_ExitThreadContext
    call k_PS2_MouseInterruptHandler
    call k_proc_EnterThreadContext
    iret