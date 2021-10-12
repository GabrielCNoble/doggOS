#ifndef K_CPU_H
#define K_CPU_H

#include "../k_defs.h"
#include "k_defs.h"

extern void k_cpu_EnableInterrupts();

extern void k_cpu_DisableInterrupts();

extern void k_cpu_EnablePaging();

extern void k_cpu_DisablePaging();

extern uint32_t k_cpu_IsPagingEnabled();

extern void k_cpu_InvalidateTLB(uint32_t address);

extern void k_cpu_Halt();

extern void k_cpu_Lgdt(struct k_cpu_seg_desc_t *gdt, uint32_t seg_count, uint32_t next_cs);

extern void k_cpu_Ltr(uint32_t selector);

extern void k_cpu_Lcr3(uint32_t pdir);

extern uint32_t k_cpu_Rcr3();

extern fastcall void k_cpu_OutB(uint8_t value, uint16_t port);

extern fastcall void k_cpu_OutW(uint16_t value, uint16_t port);

extern fastcall void k_cpu_OutD(uint32_t value, uint16_t port);

extern fastcall uint8_t k_cpu_InB(uint16_t port); 

extern fastcall uint16_t k_cpu_InW(uint16_t port);

extern fastcall uint32_t k_cpu_InD(uint16_t port);

extern void k_cpu_WriteMSR(uint32_t reg, uint64_t value);

extern uint64_t k_cpu_ReadMSR(uint32_t reg);

#endif