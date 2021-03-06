.intel_syntax noprefix
.code16

.section .text 
.global _start 

_start:
    jmp boot

boot:
    
    cli
    cld

    mov ah, 0x42
    mov si, offset read_packet
    int 0x13
    jmp 0x0000:0x7e00
    hlt


.align 4
read_packet:
size:       .byte 0x10
res:        .byte 0x0
count:      .short 0x8
offset:     .short 0x7e00
segment:    .short 0x0000
lba:        .long 0x00000001


.fill 0x1b8 - (. - _start)
.short 0x5149
.short 0x822b
.short 0x0000

.short 0x2000
.short 0x0021
.short 0x0a83
.short 0x0208
.short 0x0800
.short 0x0000
.short 0x7800
.short 0x0000


.fill 0x1fe - (. - _start)
.short 0xaa55
    

