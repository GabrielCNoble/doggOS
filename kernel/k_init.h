#ifndef K_INIT_H
#define K_INIT_H

#include <stdint.h>
#include "k_defs.h"
#include "mem/k_mem.h"
#include "gfx/k_gfx.h"
#include "k_term.h"
#include "k_int.h"
#include "k_dev.h"
#include "k_cpu.h"
#include "k_rng.h"
#include "k_apic.h"

// #include "k_fs.h"

struct k_init_data_t
{
    struct k_mem_range_t *ranges;
    uint32_t range_count;
};

void stdcall k_Init(struct k_init_data_t *init_data);

#endif