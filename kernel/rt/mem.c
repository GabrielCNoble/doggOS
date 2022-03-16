#include "mem.h"

int32_t k_rt_StrCmp(const char *str0, const char *str1)
{
    int32_t diff = 0;

    diff = *str0;
    diff -= *str1;

    while(*str0 && *str1 && !diff)
    {
        // if(diff)
        // {
        //     break;
        // }

        str0++;
        str1++;

        diff = *str0;
        diff -= *str1;
    }

    return diff;
}