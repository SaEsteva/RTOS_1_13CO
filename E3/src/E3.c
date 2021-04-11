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
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
#define SEPARACION_MS 150
#define T_SEPARACION pdMS_TO_TICKS(SEPARACION_MS)

#define WELCOME_MSG  "Ejercicio E_3.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error al crear la cola.\r\n"
#define MSG_ERROR_MTX "Error al crear el mutex.\r\n"
#define LED_ERROR LEDR
/*==================[definiciones de datos internos]=========================*/
const gpioMap_t leds_t[] = {LEDB};
const gpioMap_t gpio_t[] = {GPIO7};
QueueHandle_t queue_tec_pulsada;
SemaphoreHandle_t mutex;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led_a( void* taskParmPtr );
void tarea_led_b( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
    printf( WELCOME_MSG );

    BaseType_t res;
    uint32_t i;

    // Crear tarea en freeRTOS
    for ( i = 0 ; i < LED_COUNT ; i++ )
    {
        res = xTaskCreate(
                  tarea_led_a,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led_a",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );

        res = xTaskCreate(
                  tarea_led_b,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led_b",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );
    }

    /* inicializo driver de teclas */
    keys_Init();

    // crear mutex!
    mutex = xSemaphoreCreateMutex( );

    // Gestion de errores de mutex
    if( mutex == NULL )
    {
        gpioWrite( LED_ERROR, ON );
        printf( MSG_ERROR_MTX );
        while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
    }

    // Crear cola
    queue_tec_pulsada = xQueueCreate( 1, sizeof( TickType_t ) );

    // Gestion de errores de colas
    if( queue_tec_pulsada == NULL )
    {
        gpioWrite( LED_ERROR, ON );
        printf( MSG_ERROR_QUE );
        while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
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
void gpio_init( void )
{
    gpioInit( GPIO7, GPIO_OUTPUT );
    gpioInit( GPIO5, GPIO_OUTPUT );
    gpioInit( GPIO3, GPIO_OUTPUT );
    gpioInit( GPIO1, GPIO_OUTPUT );
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led_a( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif = 0;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        xSemaphoreTake( mutex, portMAX_DELAY );			//abrir seccion critica

        gpioWrite( leds_t[index], ON );
        gpioWrite( gpio_t[index]+1, ON );
        vTaskDelay( xPeriodicity / 2 );		// 500 -> 0.5
        gpioWrite( leds_t[index], OFF );
        gpioWrite( gpio_t[index]+1, OFF );

        xSemaphoreGive( mutex );							//cerrar seccion critica

        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );	// 1000 -> 1
    }
}

void tarea_led_b( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif = 0;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        xQueueReceive( queue_tec_pulsada, &dif,  portMAX_DELAY );			// Esperamos tecla

        gpioWrite( leds_t[index], OFF );
        gpioWrite( gpio_t[index], OFF );
        vTaskDelay( T_SEPARACION );

        xSemaphoreTake( mutex, portMAX_DELAY );			//abrir seccion critica
        gpioWrite( leds_t[index], ON );
        gpioWrite( gpio_t[index], ON );
        vTaskDelay( dif );
        gpioWrite( leds_t[index], OFF );
        gpioWrite( gpio_t[index], OFF );
        vTaskDelay( T_SEPARACION );
        xSemaphoreGive( mutex );							//cerrar seccion critica
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
