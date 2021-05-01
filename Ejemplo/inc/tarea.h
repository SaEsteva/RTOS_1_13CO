/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/23/04
 * Version: v1.2
 *===========================================================================*/

#ifndef TASK_H_
#define TASK_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "sapi.h"
#include "keys.h"

/* public macros ================================================================= */

typedef enum
{
	INIT,
	BUTTON_0_DOWN,
	BUTTON_1_DOWN,
	BUTTONS_DOWN,
	BUTTON_0_UP,
	BUTTON_1_UP,
	BUTTONS_UP
} tarea_MEF_t;

typedef struct{
	uint16_t first_val;
	uint16_t second_val;
	bool_t printer;
	bool_t time_print;
}imprimir_t;

/* types ================================================================= */

void print(imprimir_t* print_struct);

void tarea( void* taskParmPtr );
/* methods ================================================================= */


#endif //TASK_H_
