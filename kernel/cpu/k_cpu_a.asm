.intel_syntax noprefix
.code32

.section .text


.global k_cpu_EnableInterrupts
k_cpu_EnableInterrupts:
    sti
    ret

.global k_cpu_DisableInterrupts
k_cpu_DisableInterrupts:
    cli
    ret

.global k_cpu_EnablePaging
k_cpu_EnablePaging:
    push eax
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    pop eax
    ret

.global k_cpu_DisablePaging
k_cpu_DisablePaging:
    push eax
    mov eax, cr0
    and eax, 0x7fffffff
    mov cr0, eax
    pop eax
    ret

.global k_cpu_IsPagingEnabled
k_cpu_IsPagingEnabled:
    mov eax, cr0
    and eax, 0x80000000
    ret

.global k_cpu_InvalidateTLB
k_cpu_InvalidateTLB:
    push eax
    mov eax, dword ptr [esp + 8]
    invlpg dword ptr [eax]
    pop eax
    ret

.global k_cpu_Halt
k_cpu_Halt:
    hlt
    ret

.global k_cpu_Lgdt
k_cpu_Lgdt:
    push ebp
    mov ebp, esp

    push eax
    push ebx
    push ecx

    mov eax, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebp + 12]
    mov ecx, dword ptr [ebp + 16]
    sub esp, 6
    shl ebx, 3
    dec ebx
    mov word ptr [esp], bx
    mov dword ptr [esp + 2], eax
    lgdt [esp]
    add esp, 6
    
    push ecx
    mov ecx, offset _change_cs
    push ecx
    jmp dword ptr [esp]
    _change_cs:
    pop ecx
    pop ecx

    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

.global k_cpu_Ltr
k_cpu_Ltr:
    push eax
    mov eax, dword ptr [esp + 8]
    ltr ax
    pop eax
    ret

.global k_cpu_Lidt
k_cpu_Lidt:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    mov eax, dword ptr [ebp + 8]
    mov ebx, dword ptr [ebp + 12]
    sub esp, 6
    shl ebx, 3
    dec ebx
    mov word ptr [esp], bx
    mov dword ptr [esp + 2], eax
    lidt [esp]
    add esp, 6
    pop ebx
    pop eax
    pop ebp
    ret

.global k_cpu_Lcr3
k_cpu_Lcr3:
    push eax
    mov eax, dword ptr [esp + 8]
    mov cr3, eax
    pop eax
    ret

.global k_cpu_Rcr3
k_cpu_Rcr3:
    mov eax, cr3
    ret

.global k_cpu_OutB
k_cpu_OutB:
    push eax
    mov al, cl
    out dx, al
    pop eax
    ret

.global k_cpu_OutW
k_cpu_OutW:
    push eax
    mov ax, cx
    out dx, ax
    pop eax
    ret

.global k_cpu_OutSW
k_cpu_OutSW:
    push ebp
    mov ebp, esp
    push esi
    mov esi, dword ptr [ebp + 8]
    rep outsw
    pop esi
    pop ebp
    ret

.global k_cpu_OutD
k_cpu_OutD:
    push eax
    mov eax, ecx
    out dx, eax
    pop eax
    ret

.global k_cpu_InB
k_cpu_InB:
    mov dx, cx
    in al, dx
    ret

.global k_cpu_InW
k_cpu_InW:
    mov dx, cx
    in ax, dx
    ret 

.global k_cpu_InSW
k_cpu_InSW:
    push ebp
    mov ebp, esp
    push edi
    mov edi, dword ptr [ebp + 8]  
    rep insw
    pop edi
    pop ebp
    ret

.global k_cpu_InD
k_cpu_InD:
    mov dx, cx
    in eax, dx
    ret 

.global k_cpu_WriteMSR
k_cpu_WriteMSR:
    mov ecx, dword ptr [esp + 4]
    mov eax, dword ptr [esp + 8]
    mov edx, dword ptr [esp + 12]
    wrmsr 
    ret

.global k_cpu_ReadMSR
k_cpu_ReadMSR:
    mov ecx, dword ptr [esp + 4]
    rdmsr
    ret

.global k_cpu_SwitchModes
k_cpu_SwitchModes:
    ret
    
.global k_cpu_GetCPR
k_cpu_GetCPR:
    mov ax, cs
    and eax, 0x3
    ret
    
.global k_cpu_MemFence
k_cpu_MemFence:
    mfence
    ret