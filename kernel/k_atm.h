#ifndef K_ATM_H
#define K_ATM_H

#include <stdint.h>

typedef uintptr_t k_atm_spnl_t;

extern uint32_t k_atm_Xcgh32(uint32_t *location, uint32_t new);

extern uint16_t k_atm_Xcgh16(uint16_t *location, uint16_t new);

extern uint8_t k_atm_Xcgh8(uint8_t *location, uint8_t new);

extern uint32_t k_atm_CmpXcgh32(uint32_t *location, uint32_t new, uint32_t *old);

extern uint32_t k_atm_CmpXcgh16(uint16_t *location, uint16_t new, uint16_t *old);

extern uint32_t k_atm_CmpXcgh8(uint8_t *location, uint8_t new, uint8_t *old);

extern void k_atm_SpinLock(k_atm_spnl_t *lock);

extern uint32_t k_atm_TrySpinLock(k_atm_spnl_t *lock);

extern void k_atm_SpinUnlock(k_atm_spnl_t *lock);



#endif