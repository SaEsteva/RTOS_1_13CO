/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef PRINCIPAL_H_
#define PRINCIPAL_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"


#define N_MODE	11
#define N_MODE2	20

#define T1_MAX_TICKS     	pdMS_TO_TICKS(500)



#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error creating the queue.\r\n"
#define MSG_ERROR_SEM "Error creating the semafore.\r\n"
#define LED_ERROR LEDR

typedef struct
{
    gpioMap_t led;
    xQueueHandle queue; //almacenara el evento en una cola
} t_tecla_led;

typedef enum{
	MODO1_SEC,
	MODO2_SEC,
	MODO3_SEC,
	MODOOFF_SEC,
	MODOREST_SEC,
	MODOERROR_SEC
}t_modo_status;

typedef struct
{
    t_modo_status actual_status;
    t_modo_status last_status;
} t_led_struct;

/* public macros ================================================================= */
void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data );
void user_keys_event_handler_button_release( t_key_isr_signal* event_data );
void task_led( void* taskParmPtr );
void task_button( void* taskParmPtr );
void change_status(t_modo_status status,int n_reset);
void principal_init( void );


#endif
