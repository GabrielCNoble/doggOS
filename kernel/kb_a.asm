.intel_syntax noprefix
.code32

.section .text
.global k_kb_KeyboardHandler_a
k_kb_KeyboardHandler_a:
    sti
    pusha
    call k_kb_KeyboardHandler
    popa
    iret