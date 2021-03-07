.intel_syntax noprefix
.code32

.section .text

.global k_outportb
.type k_outportb, @function
k_outportb:
    mov al, cl
    out dx, al
    ret

.global k_outportw
.type k_outportw, @function
k_outportw:
    mov ax, cx
    out dx, ax
    ret

.global k_outportd
.type k_outportd, @function
k_outportd:
    mov eax, ecx
    out dx, eax
    ret

.global k_inportb
.type k_inportb, @function
k_inportb:
    mov dx, cx
    in al, dx
    ret

.global k_inportw
.type k_inportw, @function
k_inportw:
    mov dx, cx
    in ax, dx
    ret 

.global k_inportd
.type k_inportd, @function
k_inportd:
    mov dx, cx
    in eax, dx
    ret 

    