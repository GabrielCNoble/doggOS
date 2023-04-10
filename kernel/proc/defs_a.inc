

.equ k_proc_core_state_tss, 0
.equ k_proc_core_state_current_thread, 4
.equ k_proc_core_state_next_thread, 8
.equ k_proc_core_state_scheduler_thread, 12
/* .equ k_proc_core_state_delele_list, 16 */

.equ k_proc_thread_init_entry_point, 0
.equ k_proc_thread_init_user_stack, 12
.equ k_proc_thread_init_user_data, 16

.equ k_proc_thread_start_sp, 0
.equ k_proc_thread_current_sp, 4
.equ k_proc_thread_current_pmap, 8
.equ k_proc_thread_kernel_stack_offset, 12
.equ k_proc_thread_entry_point, 16
.equ k_proc_thread_return_data, 20
.equ k_proc_thread_process, 24

.equ k_proc_process_page_map, 0

.equ k_proc_r0_data_seg, 0x00000008
.equ k_proc_r0_code_seg, 0x00000010
.equ k_proc_r3_data_seg, 0x00000018
.equ k_proc_r3_code_seg, 0x00000020


.equ k_proc_shared_data_address, 0x00004000
.equ k_proc_shared_data_kernel_pmap, 0
.equ k_proc_shared_data_gdt, 4
