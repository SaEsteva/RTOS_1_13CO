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
#include "queue.h"
#include "semphr.h"

#include "FreeRTOSConfig.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
#define SEPARACION_MS 150
#define T_SEPARACION pdMS_TO_TICKS(SEPARACION_MS)

#define WELCOME_MSG  "Ejercicio E_4.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error al crear la cola.\r\n"
#define MSG_ERROR_MTX "Error al crear el mutex.\r\n"
#define LED_ERROR LEDR

#define N_QUEUE 	5
#define MSG_LENGTH 20
/*==================[definiciones de datos internos]=========================*/
const gpioMap_t leds_t[] = {LEDB};
const gpioMap_t gpio_t[] = {GPIO7};
QueueHandle_t queue_A_B;
QueueHandle_t queue_B_A;
SemaphoreHandle_t mutex;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init(void);
/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void tarea_A( void* taskParmPtr );
void tarea_B( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
	boardConfig();									// Inicializar y configurar la plataforma

	gpio_init();

	debugPrintConfigUart( USED_UART , UART_RATE );		// UART for debug messages
	printf( WELCOME_MSG );

	BaseType_t res;
	uint32_t i;

    // Crear tarea en freeRTOS
	for (i = 0 ; i < LED_COUNT ; i++)
	{
		res = xTaskCreate(
			tarea_A,                     // Funcion de la tarea a ejecutar
			( const char * )"tarea_A",   // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
			i,                          // Parametros de tarea
			tskIDLE_PRIORITY+1,         // Prioridad de la tarea
			0                           // Puntero a la tarea creada en el sistema
		);

		// Gestion de errores
		configASSERT( res == pdPASS );

		res = xTaskCreate(
			tarea_B,                     // Funcion de la tarea a ejecutar
			( const char * )"tarea_B",   // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
			i,                          // Parametros de tarea
			tskIDLE_PRIORITY+1,         // Prioridad de la tarea
			0                           // Puntero a la tarea creada en el sistema
		);

		// Gestion de errores
		configASSERT( res == pdPASS );
	}

    // crear mutex!
    mutex = xSemaphoreCreateMutex( );

    // Gestion de errores de mutex
	if( mutex == NULL)
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_MTX );
		while(TRUE);						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

    // Crear cola
	queue_A_B = xQueueCreate( N_QUEUE , sizeof(char[MSG_LENGTH]) );
	queue_B_A = xQueueCreate( N_QUEUE , sizeof(char[MSG_LENGTH]) );

	// Gestion de errores de colas
	if( (queue_A_B || queue_B_A ) == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_QUE );
		while(TRUE);						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

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
void tarea_A( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity =  LED_RATE;
	TickType_t xLastWakeTime = xTaskGetTickCount();
    // ---------- REPETIR POR SIEMPRE --------------------------

	char mensaje[MSG_LENGTH];

	xQueueSend( queue_A_B , "Comenzamos!" ,  portMAX_DELAY );

	while( TRUE )
	{
		xQueueReceive( queue_B_A , &mensaje,  portMAX_DELAY );

		gpioWrite( GPIO7 , OFF );
		gpioWrite( GPIO5 , ON );

		xSemaphoreTake( mutex , portMAX_DELAY );			//abrir seccion critica
		printf("A] Tarea B dijo: %s\r\n",mensaje);
		xSemaphoreGive( mutex );							//cerrar seccion critica

		gpioWrite( LED2, ON );
		vTaskDelay( LED_RATE / 2 );
		xQueueSend( queue_A_B , "Hola, soy A" ,  portMAX_DELAY );
		gpioWrite( LED2 , OFF );

		vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
	}
}

void tarea_B( void* taskParmPtr )
{
    // ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity =  LED_RATE;
	TickType_t xLastWakeTime = xTaskGetTickCount();
    // ---------- REPETIR POR SIEMPRE --------------------------

	char mensaje[MSG_LENGTH];

	while( TRUE )
	{
		xQueueReceive( queue_A_B , &mensaje,  portMAX_DELAY );

		gpioWrite( GPIO5 , OFF );
		gpioWrite( GPIO7 , ON );

		xSemaphoreTake( mutex , portMAX_DELAY );			//abrir seccion critica
		printf("B] Tarea A dijo: %s\r\n",mensaje);
		xSemaphoreGive( mutex );							//cerrar seccion critica

		gpioWrite( LED3, ON );
		vTaskDelay( LED_RATE / 2 );
		xQueueSend( queue_B_A , "Hola, soy B" ,  portMAX_DELAY );
		gpioWrite( LED3 , OFF );

		vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
	}
}


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
	printf( "Malloc Failed Hook!\n" );
	configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
