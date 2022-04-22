#include "string.h"

static uint32_t k_rt_itoa_masks[][10] = 
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

static unsigned char k_rt_nibble_lut[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int32_t k_rt_StrCmp(const char *str0, const char *str1)
{
    int32_t diff = 0;

    diff = *str0;
    diff -= *str1;

    while(*str0 && *str1 && !diff)
    {
        str0++;
        str1++;

        diff = *str0;
        diff -= *str1;
    }

    return diff;
}

const char *k_rt_StrStr(const char *search_in, const char *search_for)
{
    // uint32_t index = 0;
    const char *match = NULL;
    
    if(search_in && search_for)
    {
        uint32_t search_in_index = 0;
        uint32_t search_for_index = 0;
        
        // uint32_t matching = 0;
        
        while(search_in[search_in_index])
        {
            if(search_in[search_in_index] == search_for[search_for_index])
            {
                match = search_in + search_in_index;
                
                while(search_in[search_in_index] && search_in[search_in_index] == search_for[search_for_index])
                {
                    search_in_index++;
                    search_for_index++;
                }
                
                if(!search_for[search_for_index])
                {
                    break;
                }
                
                search_for_index = 0;
                match = NULL;
            }
            else
            {
                search_in_index++;
            }
        }
    }
    
    return match;
}

int32_t k_rt_StrLen(const char *str)
{
    uint32_t cursor = 0;
    
    if(str)
    {
        while(str[cursor])
        {
            cursor++;
        }
    }

    return cursor;
}

enum result_t k_rt_AtoI(const char *str, int32_t *value)
{
    uint32_t cursor = 0;
    int32_t local_value = 0;
    
    if(str)
    {        
        // while(str[cursor] == ' ')
        // {
        //     cursor++;
        // }
        // 
        // uint32_t negate = 0;
        
        
        
        while(str[cursor])
        {
            if(str[cursor] < '0' || str[cursor] > '9')
            {
                return RESULT_BAD_PARAM;
            }
            
            local_value *= 10;
            local_value += str[cursor] - '0';
            cursor++;
        }
        
        *value = local_value;
        
        return RESULT_OK;
    }

    return RESULT_BAD_PARAM;
}

enum result_t k_rt_ItoA(int32_t value, char *buffer, size_t buffer_size, size_t *result_size)
{
    size_t mask_index = 0;
    size_t buffer_length;
    size_t sign = 0;
    
    uint32_t uvalue = 0;
    uint32_t index = 0;
    uint32_t digit = 0;
    
    if(buffer && buffer_size > 0)
    {
        buffer_length = buffer_size;
        sign = value < 0;
        
        if((uint32_t)value > 0x80000000)
        {
            /* avoid signed overflow */
            value = -value;
        }
        
        uvalue = (uint32_t)value;
        
        while(uvalue < k_rt_itoa_masks[mask_index][1] && mask_index < 9)
        {
            /* find out in which power of ten we need to start */
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
                digit = uvalue / k_rt_itoa_masks[mask_index][1];
                buffer[index++] = '0' + digit;
                uvalue -= k_rt_itoa_masks[mask_index][digit];
                mask_index++;
            }

            buffer[index] = '\0';
            
            *result_size = index;
        }
        else
        {
            /* we don't have enough space in the output buffer for the conversion */
            return RESULT_OUT_OF_SPACE;
        }
        
        return RESULT_OK;
    }

    return RESULT_BAD_PARAM;
}