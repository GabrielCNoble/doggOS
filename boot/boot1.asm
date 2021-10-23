.intel_syntax noprefix
.code16

.section .text 
.global _start 

_start:
    jmp stage0_start

stage0_start:
    
    cli
    cld

    /* store the drive we're coming from */
    push edx
    mov ah, 0x42
    mov si, offset read_packet
    int 0x13
    pop edx
    jmp 0x0500

.balign 4

read_packet:
size:       .byte 0x10
res:        .byte 0x0
count:      .short 0x0002
offset:     .short 0x0500
segment:    .short 0x0000
lba:        .quad 0x00000001

/* pad until the last two bytes in this 512 bytes block. Those bytes are the boot sector signature bytes */
.fill 0x1fe - (. - _start)
.short 0xaa55



