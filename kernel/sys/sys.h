#ifndef SYS_H
#define SYS_H

#include <stdint.h>

void k_sys_Init();

void k_sys_HaltAndCatchFire(uint32_t exception, ...);

#endif