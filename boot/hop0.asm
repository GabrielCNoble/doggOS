.intel_syntax noprefix
.code16

.section .text 
.global _start 

_start:
    jmp hop0_start

hop0_start:
    
    cli
    cld

    mov ah, 0x42
    mov si, offset h0_read_packet
    int 0x13
    jmp 0x0fff:hop1_start
    hlt

.balign 4
.global h0_read_packet
h0_read_packet:
size:       .byte 0x10
res:        .byte 0x0
count:      .short 0x0001
offset:     .short hop1_start
segment:    .short 0x0fff
lba:        .quad 0x00000001

.fill 0x1fe - (. - _start)
.short 0xaa55
