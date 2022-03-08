#include "k_string.h"
#include <stdarg.h>

static const uint32_t k_itoa_masks[][10] = 
{
    { 0         , 1000000000, 2000000000, 3000000000, 4000000000, 0         , 0         , 0         , 0         , 0         },
    { 0         , 100000000 , 200000000 , 300000000 , 400000000 , 500000000 , 600000000 , 700000000 , 800000000 , 900000000 },
    { 0         , 10000000  , 20000000  , 30000000  , 40000000  , 50000000  , 60000000  , 70000000  , 80000000  , 90000000  },
    { 0         , 1000000   , 2000000   , 3000000   , 4000000   , 5000000   , 6000000   , 7000000   , 8000000   , 9000000   },
    { 0         , 100000    , 200000    , 300000    , 400000    , 500000    , 600000    , 700000    , 800000    , 900000    },
    { 0         , 10000     , 20000     , 30000     , 40000     , 50000     , 60000     , 70000     , 80000     , 90000     },
    { 0         , 1000      , 2000      , 3000      , 4000      , 5000      , 6000      , 7000      , 8000      , 9000      },
    { 0         , 100       , 200       , 300       , 400       , 500       , 600       , 700       , 800       , 900       },
    { 0         , 10        , 20        , 30        , 40        , 50        , 60        , 70        , 80        , 90        },
    { 0         , 1         , 1         , 1         , 1         , 1         , 1         , 1         , 1         , 1         },
};

static const unsigned char k_nibble_lut[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

uint32_t k_strlen(char *buf)
{
    uint32_t index = 0;
    while(buf[index] != '\0') index++;
    return index;
}

uint32_t k_itoa(char *buffer, int32_t buffer_size, int32_t value)
{
    uint32_t mask_index = 0;
    uint32_t buffer_length;
    uint32_t uvalue = 0;
    uint32_t index = 0;
    uint32_t digit = 0;
    uint32_t sign = 0;

    if(buffer_size > 0)
    {
        buffer_length = (uint32_t)buffer_size;
        sign = value < 0;
        
        if((uint32_t)value > 0x80000000)
        {
            value = -value;
        }
        
        uvalue = (uint32_t)value;
        
        while(uvalue < k_itoa_masks[mask_index][1] && mask_index < 9)
        {
            mask_index++;
        }

        buffer_length--;

        if(sign + 10 - mask_index <= buffer_length)
        {
            /* we have enough space, ignoring the null terminator (it has been accounted for) */
            if(sign)
            {
                buffer[index++] = '-';
            }

            while(mask_index < 10 && index < buffer_length)
            {
                digit = uvalue / k_itoa_masks[mask_index][1];
                buffer[index++] = '0' + digit;
                uvalue -= k_itoa_masks[mask_index][digit];
                mask_index++;
            }

            buffer[index] = '\0';
        }
    }

    return index;
}

uint32_t k_xtoa(char *buffer, int32_t buffer_size, uint64_t value)
{
    uint64_t mask = 0xf000000000000000;
    int32_t index = 0;
    uint32_t len = 0;
    
    // unsigned char temp_buffer[9];

    if(buffer_size > 0)
    {
        while(!(value & mask) && mask)
        {
            /* find out where the first non-zero nibble is. If the number is all
            zero nibbles, char_index will point at the last nibble (consequently,
            the last character of the string) */
            mask >>= 4;
            index++;
        }

        /* index of the null terminator. The converted 32 bit value
        will have at most 8 characters. */
        index = (16 - index);
        if(!index)
        {
            index++;
        }
        len = index;
        if(index < buffer_size)
        {
            /* we got enough space to fit the string */
            buffer[index] = '\0';
            do
            {
                /* scan from ls to ms nibble */
                index--;
                buffer[index] = k_nibble_lut[value & 0xf];
                value >>= 4;
            }
            while(value);
        }
    }

    return len;
}

uint32_t k_ftoa(char *buffer, int32_t buffer_size, float value)
{
    (void)buffer;
    (void)buffer_size;
    (void)value;
    return 0;
}

uint32_t k_strcat(char *buffer, int32_t buffer_size, char *str)
{
    uint32_t length;
    uint32_t buffer_length;
    uint32_t index = 0;

    if(buffer_size > 0)
    {
        buffer_length = (uint32_t)buffer_size;
        length = k_strlen(str) + 1;
        index = k_strlen(buffer);

        if(index + length < buffer_length)
        {
            buffer += index;
            uint32_t str_index = 0;

            while(str_index < length)
            {
                buffer[str_index] = str[str_index];
                str_index++;
            }

            index += length;
        }
    }

    return index;
}

void k_sfmt(char *buffer, int32_t buffer_size, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    k_vasfmt(buffer, buffer_size, fmt, args);
    va_end(args);
}

enum K_FMT_MODIFIER_TYPE
{
    K_FMT_MODIFIER_NONE = 0,
    K_FMT_MODIFIER_LONG,
    K_FMT_MODIFIER_LONG_LONG
};

void k_vasfmt(char *buffer, int32_t buffer_size, char *fmt, va_list args)
{
    uint32_t in_index = 0;
    uint32_t out_index = 0;
    uint64_t qword_arg = 0;
    uint32_t dword_arg = 0;
    uint32_t modifier = K_FMT_MODIFIER_NONE;
    uint32_t buffer_length;
    char *str_arg;
    
    if(buffer_size > 0)
    {
        /* account for null terminator. This will tell all the subsequent code how much space they have in the buffer, and will guarantee
        we'll have space for the null terminator */
        buffer_length = (uint32_t)buffer_size;
        buffer_length--;

        while(fmt[in_index] && out_index < buffer_length)
        {
            buffer[out_index] = '\0';

            if(fmt[in_index] == '%')
            {
                in_index++;

                switch(fmt[in_index])
                {
                    case 'l':
                        in_index++;
                        if(fmt[in_index] == 'l')
                        {
                            in_index++;
                            modifier = K_FMT_MODIFIER_LONG_LONG;
                        }
                        else
                        {
                            modifier = K_FMT_MODIFIER_LONG;
                        }
                    break;
                }

                switch(fmt[in_index])
                {
                    case 'd':
                        dword_arg = va_arg(args, uint32_t);
                        out_index += k_itoa(buffer + out_index, buffer_length - out_index, dword_arg);
                    break;

                    case 'x':
                        if(modifier == K_FMT_MODIFIER_LONG_LONG)
                        {
                            qword_arg = va_arg(args, uint64_t);
                        }
                        else
                        {
                            qword_arg = va_arg(args, uint32_t);
                        }

                        out_index += k_xtoa(buffer + out_index, buffer_length - out_index, qword_arg);
                    break;

                    case 's':
                        str_arg = va_arg(args, char *);
                        uint32_t append_length = k_strcat(buffer + out_index, buffer_length - out_index, str_arg);

                        if(append_length)
                        {
                            /* k_strcat returns the length of the string, including the null terminator, 
                            and places a null char after the end of the string. To avoid having this
                            terminator accidentally terminate the resulting string, we back up one char,
                            so it gets properly overwritten by anything that comes after this string. */
                            out_index += append_length - 1;
                        }
                    break;

                    case 'c':
                        dword_arg = va_arg(args, uint32_t) & 0xff;
                        buffer[out_index] = dword_arg;
                        out_index++;
                    break;
                }
                in_index++;
            }
            else
            {
                buffer[out_index++] = fmt[in_index++];
            }
        }
        
        buffer[out_index] = '\0';
    }
}

void k_memcpy(void *dst, void *src, uint32_t size)
{
    unsigned char *in = (unsigned char *)src;
    unsigned char *out = (unsigned char *)dst;

    for(uint32_t index = 0; index < size; index++)
    {
        out[index] = in[index];
    }
}



