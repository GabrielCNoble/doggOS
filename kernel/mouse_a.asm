.intel_syntax noprefix
.code32

.section .text

.global k_mouse_MouseHandler_a
k_mouse_MouseHandler_a:
    call k_proc_ExitThreadContext
    sti
    call k_mouse_MouseHandler
    cli
    call k_proc_EnterThreadContext
    iret
    