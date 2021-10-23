#ifndef DG_STRING_H
#define DG_STRING_H

#include <stdint.h>
#include <stddef.h>

extern void ds_CopyBytes(void * restrict dst, const void * restrict src, size_t size);

extern void ds_FillBytes(void * dst, uint32_t value, size_t size);

extern int32_t ds_CompareBytes(const void *a, const void *b, size_t size);

extern int32_t ds_CompareStrings(const char *a, const char *b, size_t size);

// size_t ds_Strlen(char *str);

#endif