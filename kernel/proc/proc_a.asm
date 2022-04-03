.intel_syntax noprefix
.code32

.section .text
.include "proc/defs_a.inc"

.global k_proc_SetupUserStack_a
k_proc_SetupUserStack_a:
    /* thread init */
    mov eax, dword ptr [esp + 8]
    mov ebx, dword ptr [eax + k_proc_thread_init_user_stack]
    sub ebx, 12

    /* setup a call to k_proc_StartThread */
    mov ecx, dword ptr [eax + k_proc_thread_init_user_data]
    mov dword ptr [ebx + 8], ecx
    mov ecx, dword ptr [eax + k_proc_thread_init_entry_point]
    mov dword ptr [ebx + 4], ecx
    /* k_proc_StartThread won't ever return */
    mov dword ptr [ebx], 0

    /* call k_proc_StartUserThread_a */
    int 39

.global k_proc_SetupThreadStack_a
k_proc_SetupThreadStack_a:
    mov eax, dword ptr [esp + 0]

.global k_proc_StartUserThread_a
k_proc_StartUserThread_a:
    /* eflags */
    mov eax, dword ptr [esp + 8]
    /* we need to make room for the user stack segment and stack pointer */
    sub esp, 8
    mov dword ptr [esp], offset k_proc_StartUserThread
    /* ring 3 code segment */
    mov dword ptr [esp + 4], k_proc_r3_code_seg
    /* eflags */
    mov dword ptr [esp + 8], eax
    /* user stack */
    mov dword ptr [esp + 12], ebx
    /* ring 3 stack segment */
    mov dword ptr [esp + 16], k_proc_r3_data_seg
    iret

  /* Switch between the scheduler thread and a normal thread, or between a normal thread and
  the scheduler thread */
  .global k_proc_PreemptThread_a
  k_proc_PreemptThread_a:
      call k_proc_ExitThreadContext

      mov edx, offset k_proc_core_state
      mov ecx, dword ptr [edx + k_proc_core_state_current_thread]
      mov dword ptr [ecx + k_proc_thread_current_sp], esp

      /* next thread */
      mov ebx, dword ptr [edx + k_proc_core_state_next_thread]
      cmp ebx, 0
      jne _valid_next_thread2
          /* next thread is null, so we switch to the scheduler thread */
          lea ebx, dword ptr [edx + k_proc_core_state_scheduler_thread]
      _valid_next_thread2:

      /* swap current thread for next thread, and clear next thread pointer */
      mov dword ptr [edx + k_proc_core_state_current_thread], ebx
      mov dword ptr [edx + k_proc_core_state_next_thread], 0

      /* store the start of the switch stack segment into the loaded tss. This is just useful
      for threads running out of ring 0 */
      mov ecx, dword ptr [ebx + k_proc_thread_start_sp]

      /* core tss */
      mov eax, dword ptr [edx + k_proc_core_state_tss]
      mov dword ptr [eax + 4], ecx

      /* before we can start poping stuff from the next thread stack we need to set up its
      page directory, since the stack is mapped only in it. */
      mov esp, dword ptr [ebx + k_proc_thread_current_sp]

      call k_proc_EnterThreadContext
      iret
