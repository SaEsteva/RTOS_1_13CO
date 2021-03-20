/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/18/03
 * Version: v1.0
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "UART_SO.h"
/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;


/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void sw_uart_config( uint16_t );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------
	boardConfig();									// Inicializar y configurar la plataforma

   gpioInit( GPIO7, GPIO_OUTPUT );
   gpioWrite( GPIO7 , ON ); //Inicia con el pin en alto
   
	debugPrintConfigUart( UART_USB, 115200 );		// UART for debug messages
	printf( "Ejercicio B_10.\r\n" );

	BaseType_t res;
   
   uint16_t Baurate_uart = 1600; // 200 bps para un dato de 8 bits

   // Crear tarea en freeRTOS
	res = xTaskCreate(
       sw_uart_config,                     // Funcion de la tarea a ejecutar
       ( const char * )"sw_uart_config",   // Nombre de la tarea como String amigable para el usuario
       configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
       Baurate_uart,                          // Parametros de tarea
       tskIDLE_PRIORITY+1,         // Prioridad de la tarea
       0                           // Puntero a la tarea creada en el sistema
   );

	// Gestion de errores
	configASSERT( res == pdPASS );
	    
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


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
