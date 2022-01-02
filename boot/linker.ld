ENTRY(_start)

SECTIONS
{
    .boot1 0x7c00 : AT (0x0000)
    {
        ./boot1.o
    }

    .boot2 0x0500 : AT (0x0200)
    {
        ./boot2.o
    }
}