.intel_syntax noprefix
.code32

.include "proc/defs_a.inc"

.section .text
.global k_kb_KeyboardHandler_a
k_kb_KeyboardHandler_a:
    call k_proc_ExitThreadContext
    sti
    call k_kb_KeyboardHandler
    cli
    call k_proc_EnterThreadContext
    iret
