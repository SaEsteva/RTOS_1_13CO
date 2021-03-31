/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "keys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/*=====[ Definitions of private data types ]===================================*/

/*=====[Definition macros of private constants]==============================*/
#define DEBOUNCE_TIME           40
#define DEBOUNCE_TIME_TICKS     pdMS_TO_TICKS(DEBOUNCE_TIME)

#define C1_MIN                  100
#define C1_MIN_TICKS            pdMS_TO_TICKS(C1_MIN)
#define C1_MAX                  900
#define C1_MAX_TICKS            pdMS_TO_TICKS(C1_MAX)
#define C1_INICIAL_TICKS        pdMS_TO_TICKS(500)
/*=====[Prototypes (declarations) of private functions]======================*/

static void keys_reset( uint32_t index );
static void keys_event_handler_button_pressed( uint32_t index );
static void keys_event_handler_button_release( uint32_t index );


/*=====[Definitions of private global variables]=============================*/
const t_key_config  keys_config[] = { TEC1 , TEC2} ;

#define KEY_COUNT   sizeof(keys_config)/sizeof(keys_config[0])

t_key_data keys_data[KEY_COUNT];

/*=====[Definitions of public global variables]==============================*/
extern TickType_t time_control;
/*=====[prototype of private functions]=================================*/
void keys_service_task( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/
TickType_t keys_get_diff( uint32_t index )
{
	TickType_t tiempo;

	taskENTER_CRITICAL();
	tiempo = keys_data[index].time_diff;
	taskEXIT_CRITICAL();

	return tiempo;
}

void keys_clear_diff( uint32_t index )
{
	taskENTER_CRITICAL();
	keys_data[index].time_diff = KEYS_INVALID_TIME;
	taskEXIT_CRITICAL();
}

void keys_init( void )
{
	BaseType_t res;

	for ( uint32_t i = 0 ; i < KEY_COUNT ; i++ )
	{
		keys_data[i].state          = BUTTON_UP;  // Set initial state
		keys_data[i].time_down      = KEYS_INVALID_TIME;
		keys_data[i].time_up        = KEYS_INVALID_TIME;
		keys_data[i].time_diff      = KEYS_INVALID_TIME;
	}
   
   //Valor inicial del contador
   taskENTER_CRITICAL();
   time_control = C1_INICIAL_TICKS;
   taskEXIT_CRITICAL();
   
	// Crear tareas en freeRTOS
	res = xTaskCreate (
			  keys_service_task,					// Funcion de la tarea a ejecutar
			  ( const char * )"keys_service_task",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	        // Cantidad de stack de la tarea
			  0,							        // Parametros de tarea
			  tskIDLE_PRIORITY+1,			        // Prioridad de la tarea
			  0							            // Puntero a la tarea creada en el sistema
		  );

	// Gestión de errores
	configASSERT( res == pdPASS );
}

/**
   @brief Keys SM

   @param index
 */
void keys_Update( uint32_t index )
{
	switch( keys_data[index].state )
	{
		case STATE_BUTTON_UP:
			/* CHECK TRANSITION CONDITIONS */
			if( !gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_FALLING;
			}
			break;

		case STATE_BUTTON_FALLING:
			/* ENTRY */

			/* CHECK TRANSITION CONDITIONS */
			if( !gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_DOWN;

				/* ACCION DEL EVENTO !*/
				keys_event_handler_button_pressed( index );
			}
			else
			{
				keys_data[index].state = STATE_BUTTON_UP;
			}

			/* LEAVE */
			break;

		case STATE_BUTTON_DOWN:
			/* CHECK TRANSITION CONDITIONS */
			if( gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_RISING;
			}
			break;

		case STATE_BUTTON_RISING:
			/* ENTRY */

			/* CHECK TRANSITION CONDITIONS */

			if( gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_UP;

				/* ACCION DEL EVENTO ! */
				keys_event_handler_button_release( index );
			}
			else
			{
				keys_data[index].state = STATE_BUTTON_DOWN;
			}

			/* LEAVE */
			break;

		default:
			keys_reset( index );
			break;
	}
}

/*=====[Implementations of private functions]================================*/

/**
   @brief Event handler for key pressed

   @param index
 */
static void keys_event_handler_button_pressed( uint32_t index )
{
	TickType_t current_tick_count = xTaskGetTickCount();

	taskENTER_CRITICAL();
	keys_data[index].time_down = current_tick_count;
	taskEXIT_CRITICAL();
}

/**
   @brief Event handler for key release

   @param index
 */
static void keys_event_handler_button_release( uint32_t index )
{
	TickType_t current_tick_count = xTaskGetTickCount();
   TickType_t aux_time;

	taskENTER_CRITICAL();
	keys_data[index].time_up    = current_tick_count;
	keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
   aux_time = keys_data[index].time_diff;
	taskEXIT_CRITICAL();
   
   if (index == 1){
      dec_counter(aux_time);
   }
   else{
      inc_counter(aux_time);
   }
   
}


void dec_counter(TickType_t time_dec){
   TickType_t aux_tick_count;
   
   taskENTER_CRITICAL();
   aux_tick_count = time_control;
   taskEXIT_CRITICAL();
   
   if (time_dec>aux_tick_count){
      aux_tick_count = C1_MIN_TICKS;
   }
   else {
      aux_tick_count = aux_tick_count - time_dec;
      if (aux_tick_count < C1_MIN_TICKS){
      aux_tick_count = C1_MIN_TICKS;
      }
   }
      
   taskENTER_CRITICAL();
   time_control = aux_tick_count;
   taskEXIT_CRITICAL();
}

void inc_counter(TickType_t time_inc ){
   TickType_t aux_tick_count ;
   
   taskENTER_CRITICAL();
   aux_tick_count = time_control + time_inc;
   taskEXIT_CRITICAL();
   
   if ( aux_tick_count > C1_MAX_TICKS ){
      aux_tick_count = C1_MAX_TICKS;
   }
   taskENTER_CRITICAL();
   time_control = aux_tick_count;
   taskEXIT_CRITICAL();
}
/**
   @brief Restarts the button SM

   @param index
 */
static void keys_reset( uint32_t index )
{
	taskENTER_CRITICAL();
	keys_data[index].state = BUTTON_UP;
	taskEXIT_CRITICAL();
}

/*=====[Implementations of private functions]=================================*/

/**
   @brief Keys' sevice  task implementation.

   @param taskParmPtr
 */
void keys_service_task( void* taskParmPtr )
{
	while( TRUE )
	{
		for ( uint32_t i = 0 ; i < KEY_COUNT ; i++ )
		{
			keys_Update( i );
		}

		vTaskDelay( DEBOUNCE_TIME_TICKS );
	}
}
