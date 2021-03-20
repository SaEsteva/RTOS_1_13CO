/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = { LEDB };

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*==================[declaraciones de funciones internas]====================*/

void gpio_init(void);

/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
<<<<<<< HEAD
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
	boardConfig();									// Inicializar y configurar la plataforma

	gpio_init();

	debugPrintConfigUart( UART_USB, 115200 );		// UART for debug messages
	printf( "Ejercicio B_7.\r\n" );

	BaseType_t res;

	uint32_t i;

    // Crear tarea en freeRTOS
	for (i = 0 ; i < LED_COUNT ; i++)
	{
		res = xTaskCreate(
        tarea_led,                     // Funcion de la tarea a ejecutar
        ( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
        i,                          // Parametros de tarea
        tskIDLE_PRIORITY+1,         // Prioridad de la tarea
        0                           // Puntero a la tarea creada en el sistema
    );

		// Gestion de errores
		configASSERT( res == pdPASS );
	}


    // Crear tarea en freeRTOS

   res = xTaskCreate(
     tarea_led,                     // Funcion de la tarea a ejecutar
     ( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
     configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
     0,                          // Parametros de tarea
     tskIDLE_PRIORITY+1,         // Prioridad de la tarea
     0                           // Puntero a la tarea creada en el sistema
 );

		// Gestion de errores
		configASSERT( res == pdPASS );
	

    /* inicializo driver de teclas */
    keys_Init();

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/
void gpio_init(void)
{
    gpioInit( GPIO7, GPIO_OUTPUT );
    gpioInit( GPIO5, GPIO_OUTPUT );
    gpioInit( GPIO3, GPIO_OUTPUT );
    gpioInit( GPIO1, GPIO_OUTPUT );
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{
	uint32_t index = (uint32_t) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void )
{
	// ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
	TickType_t xLastWakeTime = xTaskGetTickCount();
	TickType_t dif;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
    		dif = get_diff(0);

			if( dif != KEYS_INVALID_TIME )
			{
				if (dif > LED_RATE)
					dif = LED_RATE;
				gpioWrite( leds_t[0], ON );
				vTaskDelay( dif );
				gpioWrite( leds_t[0], OFF );
				vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
			}
			else
			{
				vTaskDelay( LED_RATE );
			}
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
