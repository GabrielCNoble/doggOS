#ifndef K_ACPI_H
#define K_ACPI_H


enum K_ACPI_GLOBAL_STATES
{
    K_ACPI_GLOBAL_STATE_G0 = 0,
    K_ACPI_GLOBAL_STATE_G1,
    K_ACPI_GLOBAL_STATE_G2,
    K_ACPI_GLOBAL_STATE_G3
};



void k_acip_Init();

void k_acip_Shutdown();

#endif