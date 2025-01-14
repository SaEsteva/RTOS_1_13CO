/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef KEYS_H_
#define KEYS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1
/* types ================================================================= */
typedef enum
{
<<<<<<< HEAD
	STATE_BUTTON_UP,
	STATE_BUTTON_DOWN,
	STATE_BUTTON_FALLING,
	STATE_BUTTON_RISING
=======
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
} keys_ButtonState_t;

typedef struct
{
<<<<<<< HEAD
	gpioMap_t tecla;			//config
=======
    gpioMap_t btn;			//config
    gpioMap_t led;			//config
    gpioMap_t gpio;			//config
    SemaphoreHandle_t sem_btn; //semaforo
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
} t_key_config;

typedef struct
{
<<<<<<< HEAD
	keys_ButtonState_t state;   //variables

	TickType_t time_down;		//timestamp of the last High to Low transition of the key
	TickType_t time_up;		    //timestamp of the last Low to High transition of the key
	TickType_t time_diff;	    //variables

	SemaphoreHandle_t sem_tec_pulsada; //semaforo
=======
    keys_ButtonState_t state;   //variables

    TickType_t time_down;		//timestamp of the last High to Low transition of the key
    TickType_t time_up;		    //timestamp of the last Low to High transition of the key
    TickType_t time_diff;	    //variables
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
} t_key_data;

/* methods ================================================================= */
void keys_Init( void );
<<<<<<< HEAD
TickType_t get_diff(uint32_t index);
void clear_diff(uint32_t index);
=======
TickType_t get_diff( uint32_t index );
void clear_diff( uint32_t index );
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe

#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
