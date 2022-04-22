#ifndef K_RT_STRING_H
#define K_RT_STRING_H

#include "../sys/defs.h"
#include <stddef.h>

int32_t k_rt_StrCmp(const char *str0, const char *str1);

const char *k_rt_StrStr(const char *search_in, const char *search_for); 

int32_t k_rt_StrLen(const char *str);

enum result_t k_rt_AtoI(const char *str, int32_t *value);

enum result_t k_rt_ItoA(int32_t value, char *buffer, size_t buffer_size, size_t *result_size);

#endif