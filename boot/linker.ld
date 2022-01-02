ENTRY(_start)

SECTIONS
{
    .boot0 0x7c00 : AT(0x0000)
    {
        boot/boot0.o
    }

    .boot1 0x0500 : AT(0x0200)
    {
        boot/boot1.o
    }
}