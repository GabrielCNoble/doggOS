ENTRY(_start)

SECTIONS
{
    .k_hop0 0x7c00 : AT (0x0000) 
    {
        boot/k_hop0.o(.text)
    }

    .k_hop1 0x7e00 : AT (0x200) 
    {
        boot/k_hop1.o(.text)
        boot/k_hop1.o(.data)
    }

    .k_hop2 : 
    {
        boot/k_hop2.o(.text)
    }

    .k_main :
    {
        kernel/*(.text)
        kernel/*(.data)
        kernel/*(.bss)
        kernel/*(.rodata)
    }
}