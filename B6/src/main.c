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

#include "leds.h"

/*==================[definiciones y macros]==================================*/
#define LED_RATE_MS 500
#define LED_RATE_MAX_MS 8000

#define LED_RATE 	 pdMS_TO_TICKS(LED_RATE_MS)
#define LED_RATE_MAX pdMS_TO_TICKS(LED_RATE_MAX_MS)

/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB, LED1, LED2, LED3};
gpioMap_t gpio_t[] = {GPIO7, GPIO5, GPIO3, GPIO1};
uint32_t rate_t[] = {LED_RATE, 2*LED_RATE, 4*LED_RATE, 8*LED_RATE};

#define N_LEDS sizeof(leds_t)/sizeof(leds_t[0])

t_led leds[N_LEDS];
/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/
void leds_init( void );
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void led_task( void* taskParmPtr ); // Prendo A - Apago A
/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------

    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( UART_USB, 115200 );		// UART for debug messages
    printf( "Ejercicio B_6.\r\n" );

    leds_init();

    uint32_t i;
    // Crear tarea en freeRTOS
    for ( i = 0 ; i < N_LEDS ; i++ )
    {
        BaseType_t res =
            xTaskCreate(
                led_task,                     	// Funcion de la tarea a ejecutar
                ( const char * )"led_task",   	// Nombre de la tarea como String amigable para el usuario
                configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
                &leds[i],                          		// Parametros de tarea
                tskIDLE_PRIORITY+1,         		// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
                0                          			// Puntero a la tarea creada en el sistema
            );

        configASSERT( res == pdPASS ); // gestion de errores
    }

    // Iniciar scheduler
    vTaskStartScheduler(); // Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

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

void leds_init( void )
{

    uint32_t i;

    for ( i = 0 ; i < N_LEDS ; i++ )
    {
        leds[i].led = leds_t[i];
        leds[i].rate = rate_t[i];
        leds[i].gpio = gpio_t[i];
    }
}

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea

void led_task( void* taskParmPtr )
{
    t_led* led = ( t_led* ) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity =  2*led->rate;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        gpioWrite( led->led, ON );
        gpioWrite( led->gpio, ON );
        vTaskDelay( led->rate );

        gpioWrite( led->led,OFF );
        gpioWrite( led->gpio, OFF );
        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
    }
}


/*==================[fin del archivo]========================================*/
