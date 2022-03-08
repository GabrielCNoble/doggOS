ENTRY(_start)

SECTIONS
{
    .boot1 0x7c00 : AT (0x0000) 
    {
        boot/boot1.o
    }

    .boot2 0x0500 : AT (0x0200)
    {
        boot/boot2.o
    }

    .bootinfo 0x7c00: AT (0x800)
    {
        kernel/k_bootinfo.o
    }

    . = ALIGN(0x200);
    k_kernel_sector = . / 0x200;
    .kernel 0x7c00 : AT (k_kernel_sector * 0x200) 
    {
        k_kernel_start = .;
        kernel/init/k_init_a.o
        kernel/*
        libdgc/*
        k_kernel_end = .;
    }

    k_kernel_size = k_kernel_end - k_kernel_start;
}