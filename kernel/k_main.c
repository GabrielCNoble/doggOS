#include "k_main.h"
#include "k_atm.h"
#include "k_apic.h"
#include "k_proc.h"

void k_main()
{
    k_printf("OK!\n");
    k_cpu_EnableInterrupts();
    k_term_clear();

    // k_term_clear();
    asm volatile
    (
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );
    k_proc_CreateThread(func1);
    k_proc_CreateThread(func2);
    k_proc_CreateThread(func3);
    k_proc_RunScheduler();
    // k_apic_StartTimer(0x3ffffff);
    // k_printf("blah\n");

    // k_apic_FireInterrupt(K_APIC_INTERRUPT_DEST_SELF, K_INT_HANDLER_TIMOUT);

    // while(1);
    // {
        // uint32_t count = k_apic_ReadReg(K_APIC_REG_CUR_COUNT);

        // if(count)
        // {
        //     // k_printf("\r%x                      ", count);
        // }
        // else
        // {
        //     for(uint32_t i = 0; i < 0xfffff; i++);
        //     k_apic_StartTimer(0xffffff);
        // }
    // }

    // k_apic_StartTimer(0xfff);
    // k_apic_StartTimer(0xffff);
    // k_cpu_DisableInterrupts();

    // k_atm_spnl_t spinlock = 0xff00;

    // k_printf("initial value stored in spinlock: %x\n", spinlock);
    
    // k_atm_SpinLock(&spinlock);
    // k_printf("locked spinlock: %x\n", spinlock);

    // k_printf("trying to lock spinlock...\n");
    // if(k_atm_TrySpinLock(&spinlock))
    // {
    //     k_printf("locked spinlock: %x\n", spinlock);
    // }
    // else
    // {
    //     k_printf("failed to lock spinlock\n");
    // }

    // k_atm_SpinUnlock(&spinlock);
    // k_printf("unlocked spinlock: %x\n", spinlock);

    // k_printf("trying to lock spinlock...\n");
    // if(k_atm_TrySpinLock(&spinlock))
    // {
    //     k_printf("locked spinlock: %x\n", spinlock);
    // }
    // else
    // {
    //     k_printf("failed to lock spinlock\n");
    // }


    // uint32_t value = 0xffffffff;
    // k_printf("value before: %x\n", value);
    // uint32_t old_value = k_atm_Xcgh8(&value, 0x0);
    // k_printf("new value: %x, old value: %x\n", value, old_value);
    // old_value = k_atm_Xcgh16(&value, 0x0);
    // k_printf("new value: %x, old value: %x\n", value, old_value);
    // old_value = k_atm_Xcgh32(&value, 0x0);
    // k_printf("new value: %x, old value: %x\n", value, old_value);

    // k_mem_MapAddress(&K_MEM_ACTIVE_MAPPED_PSTATE, 0xfee00000, 0xfee00000, K_MEM_PENTRY_FLAG_READ_WRITE);
    
    // uint32_t *apic_id = (uint32_t *)(0xfee00000 + K_APIC_REG_LOCAL_VERSION);
    // k_printf("%d\n", *apic_id);

    // uint64_t b = k_cpu_ReadMSR(0x1b);
    // k_printf("%x\n", (uint32_t)b);

    return;
}