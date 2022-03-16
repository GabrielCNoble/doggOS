#ifndef RT_MEM_H
#define RT_MEM_H

#include <stddef.h>
#include <stdint.h>

extern void k_rt_CopyBytes(void * restrict dst, const void * restrict src, size_t size);

extern void k_rt_MoveBytes(void * restrict dst, const void * restrict src, size_t size);

int32_t k_rt_StrCmp(const char *str0, const char *str1);

#endif