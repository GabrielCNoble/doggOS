.intel_syntax noprefix
.code32

.section .text

.global k_mouse_MouseHandler_a
k_mouse_MouseHandler_a:
    call k_proc_ExitThreadContext
    call k_mouse_MouseHandler
    call k_proc_EnterThreadContext
    iret
    