/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/11/04
 * Version: v1.2
 *===========================================================================*/

#ifndef MQUEUE_H_
#define MQUEUE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/* public macros ================================================================= */
//#define tec 1
//#define blink 2

enum queue_state{tec, blink};

/* types ================================================================= */
typedef struct
{
	uint8_t evento;
	gpioMap_t led;
	gpioMap_t tecla;				//config
    TickType_t tiempo_medido;	//variables
} tPrint;


#endif /* MQUEUE_H_ */
