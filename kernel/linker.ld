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
        ./*/share*.o
        ./sys/syscall_a.o
        ./sys/sys.o
        ./int/int.o
        ./kb*.o
        ./io*.o
        k_shared_end = .;
        ./*
        k_kernel_end = ALIGN(0x200);
    }

    k_kernel_end2 = 0x200 * 140;
    k_kernel_size = 200;
    /* k_kernel_size = 0x80; */
    /* k_kernel_size = (k_kernel_end - k_kernel_start) / 0x200; */
}
