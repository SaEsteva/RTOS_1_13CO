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
#define RATE                    2000
#define LED_RATE_TICKS          pdMS_TO_TICKS(RATE)


/*=====[Definitions of extern global functions]==============================*/
void gpio_init( void );
// Prototipo de funcion de la tarea
void task_led2( void* taskParmPtr );
void task_led3( void* taskParmPtr );
void keys_service_task( void* taskParmPtr );

/*=====[Definitions of variables]==============================*/
gpioMap_t leds_t[] = {LEDB, LED1, LED2, LED3};
gpioMap_t gpio_t[] = {GPIO7, GPIO5, GPIO3, GPIO1};
/*=====[Main function, program entry point after power on or reset]==========*/
extern t_key_config* keys_config;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])

TickType_t time_control;

int main( void )
{
	BaseType_t res;
   
   gpio_init();
	// ---------- CONFIGURACIONES ------------------------------
	boardConfig();  // Inicializar y configurar la plataforma

	printf( "Ejercicio D4\n" );

	// Crear tareas en freeRTOS
	res = xTaskCreate (
			  task_led2,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_led2",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0							// Puntero a la tarea creada en el sistema
		  );

	// Gestión de errores
	configASSERT( res == pdPASS );
   
   // Crear tareas en freeRTOS
	res = xTaskCreate (
			  task_led3,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_led3",	// Nombre de la tarea como String amigable para el usuario
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

//Tarea 2
void task_led2( void* taskParmPtr )
{
   
	TickType_t xLastWakeTime = xTaskGetTickCount();
   TickType_t aux_time ;

	while( 1 )
	{
		taskENTER_CRITICAL();
      aux_time = time_control;
      taskEXIT_CRITICAL();
      
      
      gpioWrite( leds_t[0], ON );
		gpioWrite( gpio_t[0], ON );
		vTaskDelay( aux_time / 2 );
		gpioWrite( leds_t[0], OFF );
		gpioWrite( gpio_t[0], OFF );
		

		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, aux_time );
	}
}

//Tarea 3
void task_led3( void* taskParmPtr )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
   TickType_t aux_time ;
   
	while( 1 )
	{
		taskENTER_CRITICAL();
      aux_time = time_control;
      taskEXIT_CRITICAL();

		gpioWrite( leds_t[1], ON );
		gpioWrite( gpio_t[1], ON );
		vTaskDelay( 2*aux_time);
		gpioWrite( leds_t[1], OFF );
		gpioWrite( gpio_t[1], OFF );
		
      //Decremento 100 el contador en cada ciclo
      dec_counter(pdMS_TO_TICKS(100));
		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, LED_RATE_TICKS );
	}
}

void gpio_init( void )
{
   uint32_t i;
   
   for ( i = 0 ; i < LED_COUNT ; i++ )
   {
        gpioInit( gpio_t[i], GPIO_OUTPUT );
   }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
