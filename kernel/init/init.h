#ifndef K_INIT_H
#define K_INIT_H

#include <stdint.h>
#include <stddef.h>
#include "../defs.h"
#include "defs.h"
#include "../mem/mem.h"
#include "../gfx/k_gfx.h"
// #include "../k_term.h"
#include "../sys/term.h"
#include "../irq/irq.h"
#include "../dev/dev.h"
#include "../dsk/dsk.h"
#include "../cpu/k_cpu.h"
#include "../k_rng.h"
#include "../timer/k_timer.h"
#include "../proc/proc.h"
#include "../sys/sys.h"
#include "../dev/kb.h"
// #include "../mouse.h"

void stdcall k_Init(struct k_init_data_t *init_data);

#endif