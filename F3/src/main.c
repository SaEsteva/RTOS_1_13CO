/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"
#include "keys.h"

/*=====[Definition & macros of public constants]==============================*/

/*=====[Definitions of extern global functions]==============================*/

// Prototipo de funcion de la tarea
void task_led( void* taskParmPtr );


typedef struct
{
	gpioMap_t led;
	xQueueHandle queue; //almacenara el evento en una cola
} t_tecla_led;


t_tecla_led leds_data[] = { {.led= LEDR}, {.led= LED1}, {.led= LED2}, {.led= LED3}};

#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])

/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	BaseType_t res;

	// ---------- CONFIGURACIONES ------------------------------
	boardConfig();  // Inicializar y configurar la plataforma

	printf( "Ejercicio F3\n" );

	for( int i = 0; i < LED_COUNT; i++ )
	{
		// Crear tareas en freeRTOS
		res = xTaskCreate (
				  task_led,					// Funcion de la tarea a ejecutar
				  ( const char * )"task_led",	// Nombre de la tarea como String amigable para el usuario
				  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
				  &leds_data[i],							// Parametros de tarea
				  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
				  0							// Puntero a la tarea creada en el sistema
			  );

		// Gestión de errores
		configASSERT( res == pdPASS );

	}

	/* inicializo driver de teclas */
	keys_init();

	// Iniciar scheduler
	vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

	/* realizar un assert con "false" es equivalente al while(1) */
	configASSERT( 0 );
	return 0;
}

void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data )
{
	//nada
}

void user_keys_event_handler_button_release( t_key_isr_signal* event_data )
{
	/* asociacion tecla - led : PODRIA HACERSE CON UNA TABLA */
	if( event_data->tecla == TEC1_INDEX )
	{
		xQueueSend( leds_data[0].queue, event_data, portMAX_DELAY );
	}

	if( event_data->tecla == TEC2_INDEX )
	{
		xQueueSend( leds_data[1].queue, event_data, portMAX_DELAY );
	}

	if( event_data->tecla == TEC3_INDEX )
	{
		xQueueSend( leds_data[2].queue, event_data, portMAX_DELAY );
	}

	if( event_data->tecla == TEC4_INDEX )
	{
		xQueueSend( leds_data[3].queue, event_data, portMAX_DELAY );
	}
}


void task_led( void* taskParmPtr )
{
	t_tecla_led* led_data = taskParmPtr;

	t_key_isr_signal evnt;
	int tecla_presionada;

	TickType_t dif =   pdMS_TO_TICKS( 500 );

		TickType_t xPeriodicity = pdMS_TO_TICKS( 1000 ); // Tarea periodica cada 1000 ms

	TickType_t xLastWakeTime = xTaskGetTickCount();

		/* creo cola para que la tarea reciba las pulsaciones de las teclas */
	led_data->queue = xQueueCreate( 2 , sizeof( t_key_isr_signal ) );

	while( 1 )
	{
		if( xQueueReceive( led_data->queue , &evnt , 0 ) == pdPASS  )
		{
			dif = keys_get_diff( evnt.tecla );
		}

		gpioWrite( led_data->led, ON );
		vTaskDelayUntil( &xLastWakeTime, dif );
		gpioWrite( led_data->led, OFF );


		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, 2*dif );
	}
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
