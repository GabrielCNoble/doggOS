ENTRY(_start)

SECTIONS
{
    .hop0 0x7c00 : AT (0x0000) 
    {
        boot/hop0.o(.text)
        boot/hop0.o(.data)
    }

    .hop1 0x7e00 : AT (0x0200) 
    {
        boot/hop1.o(.text)
        boot/hop1.o(.data)
    }
    
    . = 0x0500;
    k_kernel_start = .;
    .k_main : AT (0x0600) 
    {
        kernel/*(.text)
        kernel/*(.data)
        kernel/*(.bss)
        kernel/*(.rodata)
    }
    k_kernel_end = .;
}