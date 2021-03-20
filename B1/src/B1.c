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
#include "FreeRTOSConfig.h"

#include "sapi.h"

/*==================[definiciones y macros]==================================*/
#define LED_RATE_MS 500
#define LOADING_RATE_MS 250

#define LED_RATE pdMS_TO_TICKS(LED_RATE_MS)
#define LOADING_RATE pdMS_TO_TICKS(LOADING_RATE_MS)

#define MODE 1

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void heart_beat( void* taskParmPtr ); // Prendo A - Apago A
void loading_1( void* taskParmPtr );  // Prendo A - Apago A - Prendo B - Apago B .... hasta D
void loading_2( void* taskParmPtr ); // Prendo A - Prendo B - ... - Apago A - Apago B - ...

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------

    boardConfig();									// Inicializar y configurar la plataforma

    gpioInit( GPIO0, GPIO_OUTPUT ); // Puedo usar cualquiera del GPIO0 al GPIO8
    gpioInit( GPIO7, GPIO_OUTPUT );
    gpioInit( GPIO5, GPIO_OUTPUT );
    gpioInit( GPIO3, GPIO_OUTPUT );
    gpioInit( GPIO1, GPIO_OUTPUT );

    debugPrintConfigUart( UART_USB, 115200 );		// UART for debug messages
    printf( "Ejercicio B_1.\r\n" );

    //gpioWrite( LED3, ON );							// Led para dar señal de vida

#if MODE == 1
    // Crear tarea en freeRTOS
    BaseType_t res =
    xTaskCreate(
    	heart_beat,                     	// Funcion de la tarea a ejecutar
        ( const char * )"heart_beat",   	// Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
        0,                          		// Parametros de tarea
        tskIDLE_PRIORITY+1,         		// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
        0                          			// Puntero a la tarea creada en el sistema
    );
#endif

	#if MODE == 2
    // Crear tarea en freeRTOS
    BaseType_t res =
    xTaskCreate(
    	loading_1,                     	// Funcion de la tarea a ejecutar
        ( const char * )"loading_1",   	// Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
        0,                          		// Parametros de tarea
        tskIDLE_PRIORITY+1,         		// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
        0                          			// Puntero a la tarea creada en el sistema
    );
	#endif


	#if MODE == 3
    // Crear tarea en freeRTOS
    BaseType_t res =
    xTaskCreate(
    	loading_2,                     	// Funcion de la tarea a ejecutar
        ( const char * )"loading_2",   	// Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
        0,                          		// Parametros de tarea
        tskIDLE_PRIORITY+1,         		// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
        0                          			// Puntero a la tarea creada en el sistema
    );
	#endif
    // Gestion de errores
	//if(res == pdFAIL)
	//{
	//	gpioWrite( LEDR, ON );
	//	printf( "Error al crear las tareas.\r\n" );
	//	while(TRUE);						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	//}
	configASSERT( res == pdPASS ); // gestion de errores

    // Iniciar scheduler
    vTaskStartScheduler(); // Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    //while( TRUE )
    //{
        // Si cae en este while 1 significa que no pudo iniciar el scheduler
    //}
    configASSERT( 0 );

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.


    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void heart_beat( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    gpioWrite( GPIO0 , ON );
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        gpioWrite( LEDB , ON );
        vTaskDelay( LED_RATE_MS / portTICK_RATE_MS );

        gpioWrite( LEDB ,OFF );
        gpioToggle( GPIO0 );

        vTaskDelay( LED_RATE ); //NO USAR!!
    }
}

// Implementacion de funcion de la tarea
void loading_1( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    gpioMap_t led = LEDB;
    gpioMap_t gpio = GPIO7;

    gpioWrite( GPIO7 , ON );
    gpioWrite( GPIO5 , ON );
    gpioWrite( GPIO3 , ON );
    gpioWrite( GPIO1 , ON );
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        gpioWrite( led , ON );
        gpioWrite( gpio , ON );
        vTaskDelay( LOADING_RATE );

        gpioWrite( led ,OFF );
        gpioWrite( gpio , OFF );

        if (led == LED3)
        {
        	led = LEDB;
        	gpio = GPIO7;
        }
        else
        {
        	led = led + 1;
        	gpio ++;
        }

        vTaskDelay( LOADING_RATE ); //NO USAR!!
    }
}

// Implementacion de funcion de la tarea
void loading_2( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
    gpioMap_t led = LEDB;
    gpioMap_t gpio = GPIO7;

	gpioWrite( GPIO7 , ON );
	gpioWrite( GPIO5 , ON );
	gpioWrite( GPIO3 , ON );
	gpioWrite( GPIO1 , ON );
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
    	gpioToggle(led);
    	gpioToggle(gpio);

        if (led == LED3)
        {
        	led = LEDB;
        	gpio = GPIO7;
        }
        else
        {
        	led = led + 1;
        	gpio ++;
        }

        vTaskDelay( LOADING_RATE );
    }
}

/*==================[fin del archivo]========================================*/
