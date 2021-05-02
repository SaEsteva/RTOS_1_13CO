/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef TIME_TEC_H_
#define TIME_TEC_H_

#include "FreeRTOS.h"
#include "sapi.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1
#define DEBOUNCE_TIME           40
#define DEBOUNCE_TIME_TICKS     pdMS_TO_TICKS(DEBOUNCE_TIME)

#define T_MIN                  	1000
#define T_MIN_TICKS            	pdMS_TO_TICKS(T_MIN)
#define T_MAX                  	2000
#define T_MAX_TICKS            	pdMS_TO_TICKS(T_MAX)
#define T_INICIAL_TICKS     	pdMS_TO_TICKS(500)
#define T_INCREMENTAL			100
#define T_INCREMENTAL_TICKS   	pdMS_TO_TICKS(T_INCREMENTAL)

/* types ================================================================= */


/* methods ================================================================= */

void inc_counter();
void dec_counter();

#endif
