#ifndef K_DEFS_H
#define K_DEFS_H



#define fastcall __attribute__((fastcall))

#define stdcall __attribute__((stdcall))

enum K_STATUS_CODES
{
    K_STATUS_OK = 0,
    K_STATUS_FORBIDDEN,
    K_STATUS_INVALID_THREAD,
    K_STATUS_DETACHED_THREAD,
    K_STATUS_SPINLOCK_ALREADY_LOCKED,
    K_STATUS_SPINLOCK_ALREADY_UNLOCKED,
    K_STATUS_LINEAR_ADDRESS_IN_USE,
    K_STATUS_LINEAR_ADDRESS_NOT_IN_USE,
    K_STATUS_PHYSICAL_PAGE_IN_USE,
    K_STATUS_PHYSICAL_PAGE_NOT_IN_USE,
    K_STATUS_PHYSICAL_PAGE_EXCLUSIVE,
    K_STATUS_OUT_OF_PHYSICAL_MEM,
    K_STATUS_OUT_OF_VIRTUAL_MEM,
    K_STATUS_INVALID_STREAM,
    K_STATUS_EMPTY_STREAM,
    K_STATUS_BLOCKED_STREAM,
    K_STATUS_INVALID_DATA,
};

enum K_EXCEPTION_CODES
{
    K_EXCEPTION_OK = 0,
    K_EXCEPTION_GENERAL_PROTECTION_FAULT,
    K_EXCEPTION_DIVISION_BY_ZERO,
    K_EXCEPTION_PAGE_FAULT,
    K_EXCEPTION_INVALID_OPCODE,

    K_EXCEPTION_FAILED_MEMORY_ALLOCATION,
    K_EXCEPTION_UNKNOWN,
};


#endif