#ifndef K_MOUSE_H
#define K_MOUSE_H

#include "dev.h"

#define K_MOUSE_IRQ_VECTOR 12


struct k_dev_mouse_t
{
    struct k_dev_device_t base_device;
};

void k_mouse_Init();

void k_mouse_MouseHandler();


#endif