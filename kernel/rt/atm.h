#ifndef ATM_H
#define ATM_H

#include <stdint.h>

typedef uint32_t k_rt_spnl_t;

typedef uint32_t k_rt_cond_t;


/*
====================
k_rt_Xcgh[32|16|8]:

    atomically exchanges the value pointed by [location] with [new], and stores the value pointed by
    [location] before the exchange in the location pointed by [old]
====================
*/

extern void k_rt_Xchg32(uint32_t *location, uint32_t new, uint32_t *old);

extern void k_rt_Xchg16(uint16_t *location, uint16_t new, uint16_t *old);

extern void k_rt_Xchg8(uint8_t *location, uint8_t new, uint8_t *old);

/*
====================
k_rt_CmpXcgh[32|16|8]:

    atomically compare the value in [cmp] with the value pointed by [location], and exchange the value
    with [new] in case they're equal. The previous value pointed by [location] is always returned in [old].
    This serves to allow external code examine the value tested during the comparison and take further
    action based on it, in case the exchange wasn't sucessful.

    returns 1 if the exchange was succesful, 0 otherwise.
====================
*/

extern uint32_t k_rt_CmpXchg(uintptr_t *location, uintptr_t cmp, uintptr_t new, uintptr_t *old);

extern uint32_t k_rt_CmpXchg32(uint32_t *location, uint32_t cmp, uint32_t new, uint32_t *old);

extern uint32_t k_rt_CmpXchg16(uint16_t *location, uint16_t cmp, uint16_t new, uint16_t *old);

extern uint32_t k_rt_CmpXchg8(uint8_t *location, uint8_t cmp, uint8_t new, uint8_t *old);

/*
====================
k_rt_Inc[32|16|8]Wrap:

    atomically increment the value pointer by location, with wrapping based on the
    width of the value pointed by [location].

    returns the value prior to the increment.
====================
*/
extern uint32_t k_rt_Inc32Wrap(uint32_t *location);

extern uint32_t k_rt_Inc16Wrap(uint16_t *location);

extern uint32_t k_rt_Inc8Wrap(uint8_t *location);

/*
====================
k_rt_Dec[32|16|8]Wrap:

    atomically decrement the value pointer by location, without clamping.

    returns the value prior to the decrement.
====================
*/

extern uint32_t k_rt_Dec32Wrap(uint32_t *location);

extern uint32_t k_rt_Dec16Wrap(uint16_t *location);

extern uint32_t k_rt_Dec8Wrap(uint8_t *location);



extern uint32_t k_rt_Inc32Clamp(uint32_t *location, uint32_t max, uint32_t *old);

extern uint32_t k_rt_Dec32Clamp(uint32_t *location, uint32_t min, uint32_t *old);


extern uint32_t k_rt_Add32(uint32_t *location, uint32_t value, uint32_t *old);



extern uint32_t k_rt_AtomicOr32(uint32_t *location, uint32_t operand);

extern uint32_t k_rt_AtomicAnd32(uint32_t *location, uint32_t operand);
/*
====================
k_rt_SpinLock:

    locks a spinlock, blocking the current thread until the lock is acquired.
====================
*/
extern void k_rt_SpinLock(k_rt_spnl_t *lock);

extern void k_rt_SpinLockCritical(k_rt_spnl_t *lock);

/*
====================
k_rt_TrySpinLock:

    tries to locks a spinlock.

    returns 1 if lock was acquired, 0 otherwise.
====================
*/
extern uint32_t k_rt_TrySpinLock(k_rt_spnl_t *lock);

extern uint32_t k_rt_TrySpinLockCritical(k_rt_spnl_t *lock);

extern uint32_t k_rt_SpinWait(k_rt_spnl_t *lock);

/*
====================
k_rt_SpinUnlock:

    unlocks a spinlock.
====================
*/
extern void k_rt_SpinUnlock(k_rt_spnl_t *lock);

extern void k_rt_SpinUnlockCritical(k_rt_spnl_t *lock);



extern void k_rt_SignalCondition(k_rt_cond_t *condition);

extern void k_rt_ClearCondition(k_rt_cond_t *condition);

extern uint32_t k_rt_TestCondition(k_rt_cond_t *condition);

#endif
