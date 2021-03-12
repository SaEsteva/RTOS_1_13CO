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

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
	boardConfig();									// Inicializar y configurar la plataforma

	gpioInit( GPIO7, GPIO_OUTPUT );
	gpioInit( GPIO5, GPIO_OUTPUT );
	gpioInit( GPIO3, GPIO_OUTPUT );
	gpioInit( GPIO1, GPIO_OUTPUT );

	debugPrintConfigUart( UART_USB, 115200 );		// UART for debug messages
	printf( "Ejercicio B_5.\r\n" );

	BaseType_t res;
	uint32_t i;

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

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{
	uint32_t index = (uint32_t) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
	TickType_t dif;
    // ---------- REPETIR POR SIEMPRE --------------------------

	dif = get_diff(index);

	gpioWrite( LEDB+index, ON );
	gpioWrite( GPIO7+index , ON );
	vTaskDelay( dif );
	gpioWrite( LEDB+index, OFF );
	gpioWrite( GPIO7+index , OFF );

	vTaskDelete(NULL);
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
