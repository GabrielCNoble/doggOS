#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../mem/pmap.h"
#include "../../../mem/mngr.h"
#include "../../../defs.h"
#include "../../drv/82C59.h"
#include "../../drv/82C37.h"
#include "../../drv/ide.h"
#include "ide.h"
#include "isa.h"
#include "82C59.h"


uint32_t k_PIIX3_Init(uint8_t bus_index, uint8_t device_index)
{    
    k_PIIX3_ISA_Init(bus_index, device_index, K_PIIX3_PCI_TO_ISA_FUNCTION_INDEX);
    k_PIIX3_IDE_Init(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX);
    return K_STATUS_OK;
}