#ifndef K_IO_H
#define K_IO_H

#include <stdint.h>
#include "k_defs.h"
#include "k_dev.h"

extern fastcall void k_outportb(uint8_t value, uint16_t port);

extern fastcall void k_outportw(uint16_t value, uint16_t port);

extern fastcall void k_outportd(uint32_t value, uint16_t port);

extern fastcall uint8_t k_inportb(uint16_t port); 

extern fastcall uint16_t k_inportw(uint16_t port);

extern fastcall uint32_t k_inportd(uint16_t port);

#endif