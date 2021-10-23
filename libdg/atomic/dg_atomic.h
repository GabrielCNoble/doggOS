#ifndef DG_ATOMIC_H
#define DG_ATOMIC_H

#include "dg_defs.h"


/*
====================
dg_Xcgh[32|16|8]: 
    
    atomically exchanges the value pointed by [location] with [new], and stores the value pointed by 
    [location] before the exchange in the location pointed by [old]
====================
*/
extern void dg_Xcgh32(uint32_t *location, uint32_t new, uint32_t *old);

extern void dg_Xcgh16(uint16_t *location, uint16_t new, uint16_t *old);

extern void dg_Xcgh8(uint8_t *location, uint8_t new, uint8_t *old);

/*
====================
dg_CmpXcgh[32|16|8]: 
    
    atomically compare the value in [cmp] with the value pointed by [location], and exchange the value 
    with [new] in case they're equal. The previous value pointed by [location] is always returned in [old]. 
    This serves to allow external code examine the value tested during the comparison and take further 
    action based on it, in case the exchange wasn't sucessful.

    returns 1 if the exchange was succesful, 0 otherwise.
====================
*/
extern uint32_t dg_CmpXcgh32(uint32_t *location, uint32_t cmp, uint32_t new, uint32_t *old);

extern uint32_t dg_CmpXcgh16(uint16_t *location, uint16_t cmp, uint16_t new, uint16_t *old);

extern uint32_t dg_CmpXcgh8(uint8_t *location, uint8_t cmp, uint8_t new, uint8_t *old);

/*
====================
dg_Inc[32|16|8]Wrap: 

    atomically increment the value pointer by location, with wrapping based on the
    width of the value pointed by [location].

    returns the value prior to the increment.
====================
*/
extern uint32_t dg_Inc32Wrap(uint32_t *location);

extern uint32_t dg_Inc16Wrap(uint16_t *location);

extern uint32_t dg_Inc8Wrap(uint8_t *location);

/*
====================
dg_Dec[32|16|8]Wrap: 

    atomically decrement the value pointer by location, without clamping.
 
    returns the value prior to the decrement.
====================
*/
extern uint32_t dg_Dec32Wrap(uint32_t *location);

extern uint32_t dg_Dec16Wrap(uint16_t *location);

extern uint32_t dg_Dec8Wrap(uint8_t *location);



extern uint32_t dg_Inc32Clamp(uint32_t *location, uint32_t max, uint32_t *old);

extern uint32_t dg_Dec32Clamp(uint32_t *location, uint32_t min, uint32_t *old);


extern void dg_SpinLock(dg_spnl_t *lock);

extern uint32_t dg_TrySpinLock(dg_spnl_t *lock);

extern void dg_SpinUnlock(dg_spnl_t *lock);

#endif