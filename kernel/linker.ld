ENTRY(_start)

SECTIONS
{
    .bootinfo 0x7c00 : AT (0x000)
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
        libdg/*
        k_kernel_end = .;
    }

    k_kernel_size = k_kernel_end - k_kernel_start;
}