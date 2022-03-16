#ifndef K_EVENT_H
#define K_EVENT_H

#include <stdint.h>

enum K_EVENT_TYPES
{
    K_EVENT_TYPE_KEY_DOWN,
    K_EVENT_TYPE_KEY_UP
    K_EVENT_TYPE_MOUSE_DOWN,
    K_EVENT_TYPE_MOUSE_UP,
    K_EVENT_TYPE_MOUSE_MOVE,
    K_EVENT_TYPE_STOP
};

struct k_event_t
{
    uint32_t type;

    union
    {
        struct
        {
            // uint
        } keyboard;

        struct 
        {

        } mouse;
    };
};


#endif