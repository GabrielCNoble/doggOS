#include "k_int.h"
#include "k_term.h"


extern void *k_int0_a;
extern void *k_int3_a;
extern void *k_int4_a;
extern void *k_int5_a;
extern void *k_int6_a;
extern void *k_int7_a;
extern void *k_int8_a;
extern void *k_int13_a;
extern void *k_int14_a;
extern void *k_intn_a;
extern struct k_int_desc_t k_int_idt[];

void k_int_init()
{
    k_int_idt[K_INT_HANDLER_DE] = K_INT_DESCRIPTOR(&k_int0_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_BP] = K_INT_DESCRIPTOR(&k_int3_a, 0x10, K_INT_DESC_TYPE_TG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_OF] = K_INT_DESCRIPTOR(&k_int4_a, 0x10, K_INT_DESC_TYPE_TG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_BR] = K_INT_DESCRIPTOR(&k_int5_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_UD] = K_INT_DESCRIPTOR(&k_int6_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_NM] = K_INT_DESCRIPTOR(&k_int7_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_DF] = K_INT_DESCRIPTOR(&k_int8_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_GP] = K_INT_DESCRIPTOR(&k_int13_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLR_PF] = K_INT_DESCRIPTOR(&k_int14_a, 0x10, K_INT_DESC_TYPE_IG, K_INT_DESC_FLAG_32BIT);
    k_int_lidt();
}

void k_int_int0_handler(uint32_t eip, uint16_t cs)
{
    k_printf("division by zero at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_int3_handler(uint32_t eip, uint16_t cs)
{
    k_printf("breakpoint! next instruction at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_int4_handler(uint32_t eip, uint16_t cs)
{
    k_printf("overflow! next instruction at %x: %x\n", (uint32_t)cs, eip);
}

void k_int_int5_handler(uint32_t eip, uint16_t cs)
{

}

void k_int_int6_handler(uint32_t eip, uint16_t cs)
{
    k_printf("invalid instruction at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_int7_handler(uint32_t eip, uint16_t cs)
{
    k_printf("device not available at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_int8_handler()
{
    k_printf("double fault!\n");
}

void k_int_int13_handler(uint32_t error, uint32_t eip, uint16_t cs)
{
    k_printf("general protection fault at %x:%x, with error %x\n", (uint32_t)cs, eip, error);
}

void k_int_int14_handler(uint32_t address, uint32_t error, uint32_t eip, uint16_t cs)
{
    k_printf("page fault at %x:%x\n", (uint32_t)cs, eip);

    if(!(error & K_INT_PF_FLAG_NON_PAGED))
    {
        if(error & K_INT_PF_FLAG_WRITE)
        {
            k_printf("attempt to write to unpaged address %x\n", address);
        }
        else
        {
            if(error & K_INT_PF_FLAG_INSTR_FETCH)
            {
                k_printf("attempt to fetch instructions from unpaged address %x\n", address);
            }
            else
            {
                k_printf("attempt to read from unpaged address %x\n", address);
            }
        }
    }
    else if(error & K_INT_PF_FLAG_RES)
    {
        k_printf("couldn't translate address %x due to set reserved bits in paging structs", address);
    }
}

void k_int_intn_handler()
{
    k_printf("this exception has not been implemented!\n");
}
