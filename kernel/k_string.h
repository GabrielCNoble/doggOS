#ifndef K_STRING_H
#define K_STRING_H

#include <stdint.h>
#include <stdarg.h>

uint32_t k_strlen(char *buf);

uint32_t k_itoa(char *buffer, int32_t buffer_size, int32_t value);

uint32_t k_xtoa(char *buffer, int32_t buffer_size, uint64_t value);

uint32_t k_ftoa(char *buffer, int32_t buffer_size, float value);

uint32_t k_strcat(char *buffer, int32_t buffer_size, char *str);

void k_sfmt(char *buffer, int32_t buffer_size, char *fmt, ...);

void k_vasfmt(char *buffer, int32_t buffer_size, char *fmt, va_list args);

void k_memcpy(void *dst, void *src, uint32_t size);

#endif