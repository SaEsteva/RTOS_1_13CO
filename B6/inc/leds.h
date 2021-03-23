/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef _LEDS_H_
#define _LEDS_H_

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "sapi.h"

typedef struct
{
    gpioMap_t led;
    gpioMap_t gpio;
    TickType_t rate;
} t_led;


#endif /* _LEDS_H_ */
