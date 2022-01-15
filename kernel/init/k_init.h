#ifndef K_INIT_H
#define K_INIT_H

#include <stdint.h>
#include "../k_defs.h"
#include "k_defs.h"
#include "../mem/mem.h"
#include "../gfx/k_gfx.h"
#include "../k_term.h"
#include "../k_int.h"
#include "../dev/k_dev.h"
#include "../dsk/k_dsk.h"
#include "../cpu/k_cpu.h"
#include "../k_rng.h"
#include "../timer/k_timer.h"
#include "../proc/k_proc.h"


void stdcall k_Init(struct k_init_data_t *init_data);

#endif