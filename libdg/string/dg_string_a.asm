.intel_syntax noprefix
.code32

.section .text

.global ds_CopyBytes
ds_CopyBytes:
    mov edi, dword ptr [esp + 4]
    mov esi, dword ptr [esp + 8]
    mov eax, dword ptr [esp + 12]
    cmp edi, 0
    jz ds_copybytes_gtfo
    cmp esi, 0
    jz ds_copybytes_gtfo
    cmp eax, 0
    jz ds_copybytes_gtfo
    
    mov ecx, edi
    xor ecx, esi
    /* test if esi and edi have bits 0 and 1 equal. If they have, that's the best case scenario,
    because we can move move a byte/word at the start, and then blast full speed ahead with dword 
    copies for the bulk of the data */
    test ecx, 0x00000003
    jz ds_copybytes_same_align
    /* buffers are not aligned the same, so find out how their alignment differ */

    test ecx, 0x00000001
    /* worst case scenario: buffers aren't even word aligned */
    jnz ds_copybytes_byte_copy
    /* slightly less worse scenario: buffers are at least word aligned */
    jmp ds_copybytes_word_copy



    ds_copybytes_same_align:
    /* buffers are aligned the same, but they may start aligned to a byte or word */

    /* test if esi is pointing at a byte boundary */
    test esi, 0x00000001
    jz ds_copybytes_no_byte
    /* it is, so move a single byte */
    movsb
    sub eax, 1
    jz ds_copybytes_gtfo

    ds_copybytes_no_byte:
    /* test if esi is pointing at a word boundary */
    test esi, 0x00000002
    jz ds_copybytes_no_word
    /* it is, so move a single word */
    movsw
    sub eax, 2
    jz ds_copybytes_gtfo

    ds_copybytes_no_word:
    ds_copybytes_dword_copy:
    /* now it's guaranteed esi/edi will be dword aligned, and we can copy a bunch of dwords */
    mov ecx, eax
    shr ecx, 2
    rep movsd
    and eax, 0x00000003
    jz ds_copybytes_gtfo
    ds_copybytes_word_copy:
    /* copy any trailing words */
    mov ecx, eax
    shr ecx, 1
    rep movsw
    and eax, 0x00000001
    jz ds_copybytes_gtfo

    ds_copybytes_byte_copy:
    /* copy any trailing bytes */
    mov ecx, eax
    rep movsb

    ds_copybytes_gtfo:
    ret

.global dg_FillBytes
dg_FillBytes:
    mov edi, dword ptr [esp + 4]
    mov eax, dword ptr [esp + 8]
    mov ecx, dword ptr [esp + 12]

    test edi, 0x00000001
    jz ds_fillbytes_no_byte
    stosb
    sub ecx, 1
    jz ds_fillbytes_gtfo

    ds_fillbytes_no_byte:
    mov ah, al
    test edi, 0x00000002
    jz ds_fillbytes_no_word
    stosw
    sub ecx, 2
    jz ds_fillbytes_gtfo

    ds_fillbytes_no_word:
    mov bx, ax
    shl ebx, 16
    or eax, ebx
    shr ecx
    rep stosd
    test ecx, ecx
    jz ds_fillbytes_gtfo

    ds_fillbytes_gtfo:
    ret


.global dg_CompareBytes
dg_CompareBytes:
    mov edx, dword ptr [esp + 4]
    mov ebx, dword ptr [esp + 8]
    mov edi, dword ptr [esp + 12]
    cmp edi, 0
    jz dg_comparebytes_gtfo
    xor esi, esi
    xor eax, eax
    xor ecx, ecx
    dg_comparebytes_loop:
        mov al, byte ptr [edx + esi]
        mov cl, byte ptr [ebx + esi]
        sub eax, ecx
        jnz dg_comparebytes_gtfo
        inc esi
        cmp esi, edi
        jnz dg_comparebytes_loop
    dg_comparebytes_gtfo:
    ret

