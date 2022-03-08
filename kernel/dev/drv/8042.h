#ifndef K_8042_H
#define K_8042_H

#include <stdint.h>

uint32_t k_8042_TranslateScancode();

uint32_t k_8042_HandleInterrupt();

#endif