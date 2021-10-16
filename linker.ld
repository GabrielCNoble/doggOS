ENTRY(_start)

SECTIONS
{
    .boot 0x7c00 : AT (0x0000) 
    {
        boot/boot.o
    }

    k_kernel_sector = 4;

    .kernel : AT (k_kernel_sector * 0x0200) 
    {
        k_kernel_start = .;
        kernel/*
        libdg/*
        k_kernel_end = .;
    }
}