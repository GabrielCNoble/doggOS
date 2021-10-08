#include "k_int.h"
#include "k_term.h"
#include "cpu/k_cpu.h"
#include "k_rng.h"
#include "k_apic.h"


extern void *k_int0_a;
extern void *k_int2_a;
extern void *k_int3_a;
extern void *k_int4_a;
extern void *k_int5_a;
extern void *k_int6_a;
extern void *k_int7_a;
extern void *k_int8_a;
extern void *k_int13_a;
extern void *k_int14_a;
extern void *k_intn_a;
extern void *k_int32_a;
extern void *k_int33_a;
extern void *k_int34_a;
extern void *k_int35_a;
extern void *k_int36_a;
extern void *k_int38_a;
extern void *k_int69_a;
extern void *k_proc_SwitchToThread;
uint32_t blah;
struct k_int_desc_t k_int_idt[K_INT_HANDLER_LAST];

extern uint32_t vga_width;
extern uint32_t vga_height;
extern uint16_t *k_gfx_vga_cur_mem_map;

void k_int_Init()
{
    for(uint32_t exception = K_INT_HANDLER_DE; exception < K_INT_HANDLER_LAST; exception++)
    {
        k_int_idt[exception] = K_INT_DESCRIPTOR(&k_intn_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT); 
    }

    k_int_idt[K_INT_HANDLER_DE] = K_INT_DESCRIPTOR(&k_int0_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_NMI] = K_INT_DESCRIPTOR(&k_int2_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_BP] = K_INT_DESCRIPTOR(&k_int3_a, 0x10, K_INT_DESC_TYPE_TRAP_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_OF] = K_INT_DESCRIPTOR(&k_int4_a, 0x10, K_INT_DESC_TYPE_TRAP_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_BR] = K_INT_DESCRIPTOR(&k_int5_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_UD] = K_INT_DESCRIPTOR(&k_int6_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_NM] = K_INT_DESCRIPTOR(&k_int7_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_DF] = K_INT_DESCRIPTOR(&k_int8_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_GP] = K_INT_DESCRIPTOR(&k_int13_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_PF] = K_INT_DESCRIPTOR(&k_int14_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_CMCI] = K_INT_DESCRIPTOR(&k_int32_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_THERM] = K_INT_DESCRIPTOR(&k_int33_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_LINT0] = K_INT_DESCRIPTOR(&k_int34_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_LINT1] = K_INT_DESCRIPTOR(&k_int35_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_ERROR] = K_INT_DESCRIPTOR(&k_int36_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_TIMOUT] = K_INT_DESCRIPTOR(&k_int38_a, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_idt[K_INT_HANDLER_RUN_THREAD] = K_INT_DESCRIPTOR(&k_proc_SwitchToThread, 0x10, K_INT_DESC_TYPE_INT_GATE, K_INT_DESC_FLAG_32BIT);
    k_int_Lidt(k_int_idt, K_INT_HANDLER_LAST);
}

void k_int_Int0(uint32_t eip, uint16_t cs)
{
    k_printf("division by zero at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int2()
{
    k_printf("NMI!\n");
}

void k_int_Int3(uint32_t eip, uint16_t cs)
{
    k_printf("breakpoint! next instruction at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int4(uint32_t eip, uint16_t cs)
{
    k_printf("overflow! next instruction at %x: %x\n", (uint32_t)cs, eip);
}

void k_int_Int5(uint32_t eip, uint16_t cs)
{

}

void k_int_Int6(uint32_t eip, uint16_t cs)
{
    k_printf("invalid instruction at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int7(uint32_t eip, uint16_t cs)
{
    k_printf("device not available at %x:%x\n", (uint32_t)cs, eip);
}

void k_int_Int8()
{
    asm volatile
    (
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );
    
    k_printf("double fault!\n");
}

void k_int_Int13(uint32_t error, uint32_t eip, uint16_t cs)
{
    k_printf("general protection fault at %x:%x, with error %x\n", (uint32_t)cs, eip, error);
}

void k_int_Int14(uint32_t address, uint32_t error, uint32_t eip, uint16_t cs)
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

void k_int_Intn()
{
    k_printf("this exception has not been implemented!\n");
}

void k_int_Int32()
{
    k_printf("cmci\n");
}

void k_int_Int33()
{
    k_printf("therm\n");
}

void k_int_Int34()
{
    k_printf("lint0\n");
}

void k_int_Int35()
{
    k_printf("lint1\n");
}

void k_int_Int36()
{
    k_printf("error\n"); 
}

void k_int_Int38()
{
    k_apic_WriteReg(K_APIC_REG_EOI, 0);
    k_printf("\nthread preempted!\n");
}

void k_int_Int69()
{
    k_printf("\ntimer has timed out!");
    // k_apic_WriteReg(K_APIC_REG_EOI, 0);
}

void k_int_HaltAndCatchFire2()
{
    k_cpu_DisableInterrupts();
    k_term_SetColors(K_VGA_COLOR_WHITE, K_VGA_COLOR_BLUE);
    k_term_clear();
    k_printf("CRASH!\n");
    k_printf("Well, shit is fucked.\n");
    k_printf("STOP CODE: FUCK_YOU\n");
    k_cpu_Halt();
}

void k_int_HaltAndCatchFire()
{
    // k_int_HaltAndCatchFire2();

    k_cpu_DisableInterrupts();
    k_term_clear();
    k_printf("A terrible TERRIBLE thing has happened.\n");
    // k_printf("Setting stuff on fire. Please hold...\n");
    k_printf("The machine will now halt and catch fire.\n");

    uint8_t fire_buffer[80 * 25] = {};
    uint8_t rand_offset_cursor = 0;
    // uint8_t rand_offsets[] = { 2, 13, 46, 4, 9, 37, 52, 14, 9, 7, 13, 0, 5, 128, 37, 1, 63, 6, 255, 42, 51, 8, 97, 3, 6, 97, 65};
    // uint8_t rand_offsets[] = {23, 27, 6, 3, 22, 8, 27, 21, 34, 30, 6, 11, 18, 27, 30, 23, 15, 32};
    // uint8_t rand_offsets[] = {0, 3, 1, 2, 3, 0, 2, 0, 3, 1, 3, 2, 0, 1, 2, 3};
    uint8_t rand_offsets[] = {0, 3, 1, 2, 3, 0, 2, 0, 3, 1, 3, 2, 0, 1, 2, 3, 0, 3, 2, 1, 1, 2, 0, 3, 0, 1};

    uint16_t colors[] = 
    {
        k_vga_char(178, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BLACK)),
        k_vga_char(178, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BROWN)),
        k_vga_char(177, k_vga_attrib(K_VGA_COLOR_BLACK, K_VGA_COLOR_BROWN)),
        k_vga_char(177, k_vga_attrib(K_VGA_COLOR_GREEN, K_VGA_COLOR_LIGHT_RED)),
        k_vga_char(177, k_vga_attrib(K_VGA_COLOR_LIGHT_BROWN, K_VGA_COLOR_LIGHT_RED)),
        k_vga_char(219, k_vga_attrib(K_VGA_COLOR_LIGHT_BROWN, K_VGA_COLOR_LIGHT_BROWN)),
        k_vga_char(177, k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_LIGHT_BROWN)),
        k_vga_char(219, k_vga_attrib(K_VGA_COLOR_WHITE, K_VGA_COLOR_WHITE)),
    };

    for(uint32_t column_index = 0; column_index < vga_width; column_index++)
    {
        for(uint32_t row_index = 1; row_index < vga_height; row_index++)
        {
            fire_buffer[row_index * vga_width + column_index] = 0;
        }
    }

    for(uint32_t delay = 0; delay < 0x2fffffff; delay++);

    for(uint32_t column_index = 0; column_index < vga_width; column_index++)
    {
        fire_buffer[vga_width * (vga_height - 1) + column_index] = 7;
    }

    while(1)
    {
        for(uint32_t column_index = 0; column_index < vga_width; column_index++)
        {
            for(uint32_t row_index = 3; row_index < vga_height; row_index++)
            {
                uint32_t from = row_index * vga_width + column_index;

                if(fire_buffer[from] > 0)
                {
                    uint32_t rand = k_rng_Rand() % 4;
                    uint32_t to = from - vga_width - (rand % 3);
                    fire_buffer[to] = fire_buffer[from] - (rand & 1);
                }

                k_gfx_vga_cur_mem_map[vga_width * row_index + column_index] = colors[fire_buffer[vga_width * row_index + column_index]];
            }
        }

        for(uint32_t delay = 0; delay < 0xffffff; delay++);
    }

    k_cpu_Halt();
}