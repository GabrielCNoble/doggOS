#ifndef DG_ATOMIC_H
#define DG_ATOMIC_H

#include "k_defs.h"


extern void k_atm_Xcgh32(uint32_t *location, uint32_t new, uint32_t *old);

extern void k_atm_Xcgh16(uint16_t *location, uint16_t new, uint16_t *old);

extern void k_atm_Xcgh8(uint8_t *location, uint8_t new, uint8_t *old);

extern uint32_t k_atm_CmpXcgh(uintptr_t *location, uintptr_t cmp, uintptr_t new, uintptr_t *old);

extern uint32_t k_atm_CmpXcgh32(uint32_t *location, uint32_t cmp, uint32_t new, uint32_t *old);

extern uint32_t k_atm_CmpXcgh16(uint16_t *location, uint16_t cmp, uint16_t new, uint16_t *old);

extern uint32_t k_atm_CmpXcgh8(uint8_t *location, uint8_t cmp, uint8_t new, uint8_t *old);


extern uint32_t k_atm_Inc32Wrap(uint32_t *location);

extern uint32_t k_atm_Inc16Wrap(uint16_t *location);

extern uint32_t k_atm_Inc8Wrap(uint8_t *location);


extern uint32_t k_atm_Dec32Wrap(uint32_t *location);

extern uint32_t k_atm_Dec16Wrap(uint16_t *location);

extern uint32_t k_atm_Dec8Wrap(uint8_t *location);



extern uint32_t k_atm_Inc32Clamp(uint32_t *location, uint32_t max, uint32_t *old);

extern uint32_t k_atm_Dec32Clamp(uint32_t *location, uint32_t min, uint32_t *old);


extern void k_atm_SpinLock(k_atm_spnl_t *lock);

extern uint32_t k_atm_TrySpinLock(k_atm_spnl_t *lock);

extern void k_atm_SpinUnlock(k_atm_spnl_t *lock);

#endif