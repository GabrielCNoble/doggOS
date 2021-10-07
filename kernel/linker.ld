ENTRY(k_init_a)

SECTIONS
{
    .kernel 0x500: AT(0x0000)
    {
        k_kernel_start = .;
        kernel/*
        k_kernel_end = .;
    }
}