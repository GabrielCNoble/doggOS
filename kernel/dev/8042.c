#include "8042.h"
#include "../cpu/k_cpu.h"

uint8_t k_8042_ReadScancode()
{
    return k_cpu_InB(K_DEV_PS2_DATA_PORT);
}

uint8_t k_8042_ReadStatus()
{
    return k_cpu_InB(K_DEV_PS2_STATUS_CMD_PORT);
}