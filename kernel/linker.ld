ENTRY(_start)

SECTIONS
{
    .bootinfo 0x7c00 : AT (0x800)
    {
        ./boot/k_bootinfo.o
    }

    . = ALIGN(0x200);
    k_kernel_sector = . / 0x200;
    .kernel 0x7c00 : AT (k_kernel_sector * 0x200)
    {
        k_kernel_start = .;
        ./init/k_init_a.o
        . = ALIGN(0x1000);
        k_shared_start = .;
        ./proc/proc_a.o
        ./proc/thread_a.o
        ./proc/thread_e.o
        ./sys/syscall_s_a.o
        ./sys/syscall_m_a.o
        ./k_string.o
        ./int/irq.o
        ./kb*.o
        k_shared_end = .;
        ./*
        k_kernel_end = ALIGN(0x200);
    }

    k_kernel_end2 = 0x200 * 140;
    /* k_kernel_size = 200; */
    k_kernel_size = 0x80;
    /* k_kernel_size = (k_kernel_end - k_kernel_start) / 0x200; */
}
