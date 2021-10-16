#ifndef DG_ATOMIC_H
#define DG_ATOMIC_H

#include "dg_defs.h"


extern uint32_t dg_Xcgh32(uint32_t *location, uint32_t new, uint32_t *old);


extern uint32_t dg_CmpXcgh32(uint32_t *location, uint32_t cmp, uint32_t new, uint32_t *old);


extern uint32_t dg_Inc32Wrap(uint32_t *location);

extern uint32_t dg_Inc32Clamp(uint32_t *location, uint32_t max, uint32_t *old);

extern uint32_t dg_Dec32Wrap(uint32_t *location);

extern uint32_t dg_Dec32Clamp(uint32_t *location, uint32_t min, uint32_t *old);


extern void dg_SpinLock(dg_spnl_t *lock);

extern uint32_t dg_TrySpinLock(dg_spnl_t *lock);

extern void dg_SpinUnlock(dg_spnl_t *lock);

#endif