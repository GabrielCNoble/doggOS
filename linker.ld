ENTRY(_start)

SECTIONS
{
    .hop0 0x7c00 : AT (0x0000) 
    {
        boot/hop0.o(.text)
    }

    .hop1 0x7e00 : AT (0x200) 
    {
        boot/hop1.o(.text)
        boot/hop1.o(.data)
    }

    .hop2 : 
    {
        boot/hop2.o(.text)
    }

    .k_main :
    {
        kernel/*(.text)
        kernel/*(.data)
        kernel/*(.bss)
        kernel/*(.rodata)
    }
}