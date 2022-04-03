#include "ide.h"
#include "piix3.h"
#include "../../../cpu/k_cpu.h"
#include "../../../int/int.h"
#include "../../../io.h"

struct k_io_stream_t *k_PIIX3_IDE_stream;

extern void *k_PIIX3_IDE_Handler_a;

uint32_t k_PIIX3_IDE_Init(uint8_t bus_index, uint8_t device_index)
{
    // uint32_t base_address = K_PCI_DEVICE_CONFIG_ADDRESS(bus_index, device_index, K_PIIX3_IDE_INTERFACE_FUNCTION_INDEX, 0);
    k_int_SetInterruptHandler(46, &k_PIIX3_IDE_Handler_a, K_CPU_SEG_SEL(6, 3, 0), 3);
    k_PIIX3_IDE_stream = k_io_AllocStream();
    k_io_UnblockStream(k_PIIX3_IDE_stream);
    return 0;
}

uint8_t k_PIIX3_IDE_ReadStatus()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_STATUS_REG);
}

uint8_t K_PIIX3_IDE_ReadError()
{
    return k_cpu_InB(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_ERROR_REG);
}

void k_PIIX3_IDE_ExecCmd(uint8_t cmd)
{
    while(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_BSY);
    k_cpu_OutB(cmd, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CMD_REG);
}

void k_PIIX3_IDE_Delay()
{
    for(uint32_t x = 0; x < 0xffff; x++);
}

uint32_t read_done = 1;

void k_PIIX3_IDE_Read(uint32_t lba, uint32_t size)
{
    while(!(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_DRDY));
    
    // k_sys_TerminalPrintf("read start at %x, %x sectors\n", lba, size);
    // {
    k_cpu_OutB( (lba & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_NUMBER_REG);
    k_cpu_OutB( ((lba >> 8) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_LOW_REG);
    k_cpu_OutB( ((lba >> 16) & 0xff), K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_CYL_HIGH_REG);
    k_cpu_OutB( ((lba >> 24) & 0x0f) | 0xe0, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_HEAD_REG);
    k_cpu_OutB(size, K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_SEC_COUNT_REG);
    k_PIIX3_IDE_ExecCmd(K_IDE_CMD_READ_SEC_NR);
    // }
}

void k_PIIX3_IDE_Handler()
{
    uint16_t temp_data[32];
    if(k_PIIX3_IDE_ReadStatus() & K_IDE_STATUS_FLAG_ERROR)
    {
        k_sys_TerminalPrintf("disk io error: %x\n", (uint32_t)K_PIIX3_IDE_ReadError());
    }
    else
    {
        for(uint32_t index = 0; index < 256 / (sizeof(temp_data) / sizeof(temp_data[0])); index++)
        // for(uint32_t index = 0; index < 256; index++)
        {
            // uint16_t data = k_cpu_InW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG);
            k_cpu_InSW(K_PIIX3_PRIMARY_IDE_CMD_BLOCK + K_PIIX3_IDE_DATA_REG, temp_data, sizeof(temp_data) / sizeof(temp_data[0]));
            k_io_WriteStreamData(k_PIIX3_IDE_stream, temp_data, sizeof(temp_data));
        }
    }
    
    k_io_SignalStream(k_PIIX3_IDE_stream);
    
    k_PIIX3_82C59_EndOfInterrupt();
}