#ifndef K_APIC_H
#define K_APIC_H

#include <stdint.h>

enum K_APIC_REGS
{
    K_APIC_REG_LOCAL_ID = 0x0020,
    K_APIC_REG_LOCAL_VERSION = 0x0030,
    K_APIC_REG_TASK_PRIORITY = 0x0080,
    K_APIC_REG_ARB_PRIORITY = 0x0090,
    K_APIC_REG_PROC_PRIORITY = 0x00a0,
    K_APIC_REG_EOI = 0x00b0,
    K_APIC_REG_REMOTE_READ = 0x00c0,
    K_APIC_REG_LOG_DEST = 0x00d0,
    K_APIC_REG_DEST_FORMAT = 0x00e0,
    K_APIC_REG_SPUR_INT_VEC = 0x00f0,
    K_APIC_REG_IN_SERVICE0 = 0x0100,
    K_APIC_REG_IN_SERVICE1 = 0x0110,
    K_APIC_REG_IN_SERVICE2 = 0x0120,
    K_APIC_REG_IN_SERVICE3 = 0x0130,
    K_APIC_REG_IN_SERVICE4 = 0x0140,
    K_APIC_REG_IN_SERVICE5 = 0x0150,
    K_APIC_REG_IN_SERVICE6 = 0x0160,
    K_APIC_REG_IN_SERVICE7 = 0x0170,
    K_APIC_REG_TRIGGER_MODE0 = 0x0180,
    K_APIC_REG_TRIGGER_MODE1 = 0x0190,
    K_APIC_REG_TRIGGER_MODE2 = 0x01a0,
    K_APIC_REG_TRIGGER_MODE3 = 0x01b0,
    K_APIC_REG_TRIGGER_MODE4 = 0x01c0,
    K_APIC_REG_TRIGGER_MODE5 = 0x01d0,
    K_APIC_REG_TRIGGER_MODE6 = 0x01e0,
    K_APIC_REG_TRIGGER_MODE7 = 0x01f0,
    K_APIC_REG_INT_REQUEST0 = 0x0200,
    K_APIC_REG_INT_REQUEST1 = 0x0210,
    K_APIC_REG_INT_REQUEST2 = 0x0220,
    K_APIC_REG_INT_REQUEST3 = 0x0230,
    K_APIC_REG_INT_REQUEST4 = 0x0240,
    K_APIC_REG_INT_REQUEST5 = 0x0250,
    K_APIC_REG_INT_REQUEST6 = 0x0260,
    K_APIC_REG_INT_REQUEST7 = 0x0270,
    K_APIC_REG_ERROR_STATUS = 0x0280,
    K_APIC_REG_LVT_CMCI = 0x02f0,
    K_APIC_REG_INT_CMD0 = 0x0300,
    K_APIC_REG_INT_CMD1 = 0x0310,
    K_APIC_REG_LVT_TIMER = 0x0320,
    K_APIC_REG_LVT_THERM_SENSOR = 0x0330,
    K_APIC_REG_LVT_PERF_MON_COUNTERS = 0x0340,
    K_APIC_REG_LVT_LINT0 = 0x0350,
    K_APIC_REG_LVT_LINT1 = 0x0360,
    K_APIC_REG_LVT_ERROR = 0x0370,
    K_APIC_REG_INIT_COUNT = 0x0380,
    K_APIC_REG_CUR_COUNT = 0x0390,
    K_APIC_REG_DIV_CONFIG = 0x03e0,
};


#define K_APIC_INT_DEST_SELF 0xffff
#define K_APIC_INT_DEST_BROADCAST 0xff

enum K_APIC_INT_DELIVERY_MODES
{
    K_APIC_INT_DELIVERY_MODE_FIXED = 0x0,
    K_APIC_INT_DELIVERY_MODE_SMI = 0x2 << 8,
    K_APIC_INT_DELIVERY_MODE_NMI = 0x4 << 8,
    K_APIC_INT_DELIVERY_MODE_INIT = 0x5 << 8,
    K_APIC_INT_DELIVERY_MODE_STARTUP = 0x6 << 8,
    K_APIC_INT_DELIVERY_MODE_LAST = 0x7 << 8,
};

enum K_APIC_INT_TRIGGER_MODES
{
    K_APIC_INT_TRIGGER_MODE_EDGE = 0,
    K_APIC_INT_TRIGGER_MODE_LEVEL = 1 << 15,
    K_APIC_INT_TRIGGER_MODE_LAST = K_APIC_INT_TRIGGER_MODE_LEVEL
};

enum K_APIC_INT_DEST_MODES
{
    K_APIC_INT_DEST_MODE_PHYSICAL = 0,
    K_APIC_INT_DEST_MODE_LOGICAL = 1 << 11,
    K_APIC_INT_DEST_MODE_LAST = K_APIC_INT_DEST_MODE_LOGICAL
};

enum K_APIC_INT_DEST_SHORTHANDS
{
    K_APIC_INT_DEST_SHORTHAND_NONE = 0,
    K_APIC_INT_DEST_SHORTHAND_SELF = 1 << 18,
    /* all including self */
    K_APIC_INT_DEST_SHORTHAND_AINS = 2 << 18,
    /* all excluding self */
    K_APIC_INT_DEST_SHORTHAND_AEXS = 3 << 18,
    K_APIC_INT_DEST_SHORTHAND_LAST = K_APIC_INT_DEST_SHORTHAND_AEXS

};

void k_apic_Init();

void k_apic_WriteReg(uint32_t reg, uint32_t value);

void k_apic_AndReg(uint32_t reg, uint32_t value);

void k_apic_OrReg(uint32_t reg, uint32_t value);

uint32_t k_apic_ReadReg(uint32_t reg);

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

void k_apic_EndOfInterrupt();

void k_apic_FireIPInterrupt(uint16_t destination, uint8_t interrupt);

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

void k_apic_StartTimer(uint32_t count);

void k_apic_StopTimer();


#endif