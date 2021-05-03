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
#define RATE                    1000
#define LED_RATE_TICKS          pdMS_TO_TICKS(RATE)
/*=====[Definitions of extern global functions]==============================*/

// Prototipo de funcion de la tarea
void task_led( void* taskParmPtr );
void keys_service_task( void* taskParmPtr );

/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	BaseType_t res;

	// ---------- CONFIGURACIONES ------------------------------
	boardConfig();  // Inicializar y configurar la plataforma

	printf( "Ejercicio D3\n" );

	// Crear tareas en freeRTOS
	res = xTaskCreate (
			  task_led,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_led",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0							// Puntero a la tarea creada en el sistema
		  );

	// Gestión de errores
	configASSERT( res == pdPASS );

	/* inicializo driver de teclas */
	keys_init();

	// Iniciar scheduler
	vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

	/* realizar un assert con "false" es equivalente al while(1) */
	configASSERT( 0 );
	return 0;
}

void task_led( void* taskParmPtr )
{
	uint32_t index = ( uint32_t ) taskParmPtr;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		TickType_t dif = keys_get_diff( index );

		if( dif != KEYS_INVALID_TIME && dif > 0 )
		{
			if ( dif > LED_RATE_TICKS )
			{
				dif = LED_RATE_TICKS;
			}
			gpioWrite( LEDB, ON );
			vTaskDelay( dif );
			gpioWrite( LEDB, OFF );
		}

		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, LED_RATE_TICKS );
	}
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}