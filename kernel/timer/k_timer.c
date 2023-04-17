#include "k_timer.h"
#include "../irq/apic.h"

void k_timer_Init()
{
    k_apic_Init();
}