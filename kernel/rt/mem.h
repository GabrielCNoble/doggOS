#ifndef RT_MEM_H
#define RT_MEM_H

#include <stddef.h>
#include <stdint.h>

/* TODO: use vector instructions for wider than 4 byte per iteration copies */
extern void k_rt_CopyBytes(void * restrict dst, const void * restrict src, size_t size);

extern void k_rt_MoveBytes(void * restrict dst, const void * restrict src, size_t size);

extern void k_rt_SetBytes(void * restrict dst, size_t size, uint32_t value);

extern void k_rt_SetWords(void * restrict dst, size_t size, uint32_t value);

extern void k_rt_SetDwords(void * restrict dst, size_t size, uint32_t value);

#endif