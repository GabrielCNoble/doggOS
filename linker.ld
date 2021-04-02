ENTRY(_start)

SECTIONS
{
    .hop0 0x7c00 : AT (0x0000) 
    {
        boot/hop0.o
    }

    .hop1 0x7e00 : AT (0x0200) 
    {
        boot/hop1.o
    }

    k_kernel_sector = 2;
    .kernel 0x500 : AT (k_kernel_sector * 0x0200) 
    {
        k_kernel_start = .;
        kernel/*
        k_kernel_end = .;
    }
    
}