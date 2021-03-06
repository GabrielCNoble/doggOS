#ifndef K_STRING_H
#define K_STRING_H

#include <stdint.h>
#include <stdarg.h>

uint32_t k_strlen(unsigned char *buf);

uint32_t k_itoa(unsigned char *buffer, int32_t buffer_size, int32_t value);

uint32_t k_xtoa(unsigned char *buffer, int32_t buffer_size, uint64_t value);

uint32_t k_strcat(unsigned char *buffer, int32_t buffer_size, unsigned char *str);

void k_ftoa(unsigned char *buf, int32_t buf_size, uint32_t v);

void k_sfmt(unsigned char *buffer, int32_t buffer_size, unsigned char *fmt, ...);

void k_vasfmt(unsigned char *buffer, int32_t buffer_size, unsigned char *fmt, va_list args);

#endif