ENTRY(_start)

SECTIONS
{
    .bootinfo 0x7c00 : AT (0x800)
    {
        ./k_bootinfo.o
    }

    . = ALIGN(0x200);
    k_kernel_sector = . / 0x200;
    .kernel 0x7c00 : AT (k_kernel_sector * 0x200) 
    {
        k_kernel_start = .;
        ./init/k_init_a.o
        ./*
        k_kernel_end = .;
    }

    k_kernel_size = k_kernel_end - k_kernel_start;
}