.intel_syntax noprefix
.code32

.section .text

.global k_rt_CopyBytes
k_rt_CopyBytes:
    mov edi, dword ptr [esp + 4]
    mov esi, dword ptr [esp + 8]
    mov eax, dword ptr [esp + 12]

    cmp edi, 0
    jz k_rt_copybytes_gtfo
    cmp esi, 0
    jz k_rt_copybytes_gtfo
    cmp eax, 0
    jz k_rt_copybytes_gtfo
    
    mov ecx, edi
    xor ecx, esi
    /* test if esi and edi have bits 0 and 1 equal. If they have, that's the best case scenario,
    because we can move move a byte/word at the start, and then blast full speed ahead with dword 
    copies for the bulk of the data */
    test ecx, 0x00000003
    jz k_rt_copybytes_same_align
    /* buffers are not aligned the same, so find out how their alignment differ */

    test ecx, 0x00000001
    /* worst case scenario: buffers aren't even word aligned */
    jnz k_rt_copybytes_byte_copy
    /* slightly less worse scenario: buffers are at least word aligned */
    jmp k_rt_copybytes_word_copy



    k_rt_copybytes_same_align:
        /* buffers are aligned the same, but they may start aligned to a byte or word */

        /* test if esi is pointing at a byte boundary */
        test esi, 0x00000001
        jz k_rt_copybytes_no_byte
        /* it is, so move a single byte */
        movsb
        sub eax, 1
        jz k_rt_copybytes_gtfo

    k_rt_copybytes_no_byte:
        /* test if esi is pointing at a word boundary */
        test esi, 0x00000002
        jz k_rt_copybytes_no_word
        /* test if we have more than a single byte. If we do, we'll have to do a single byte copy. */
        test eax, 0xfffffffe
        jz k_rt_copybytes_byte_copy
        /* it is, so move a single word */
        movsw
        sub eax, 2
        jz k_rt_copybytes_gtfo

    /* we may have trailing words/bytes after the bulk of dwords. That's why we just fall through
    here, instead of branching */

    k_rt_copybytes_no_word:
    k_rt_copybytes_dword_copy:
        /* now it's guaranteed esi/edi will be dword aligned, and we can copy a bunch of dwords */
        mov ecx, eax
        shr ecx, 2
        rep movsd
        and eax, 0x00000003
        jz k_rt_copybytes_gtfo

    k_rt_copybytes_word_copy:
        /* copy any trailing words */
        mov ecx, eax
        shr ecx, 1
        rep movsw
        and eax, 0x00000001
        jz k_rt_copybytes_gtfo

    k_rt_copybytes_byte_copy:
        /* copy any trailing bytes */
        mov ecx, eax
        rep movsb

    k_rt_copybytes_gtfo:
        ret
