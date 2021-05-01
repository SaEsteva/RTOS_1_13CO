/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/23/04
 * Version: v1.2
 *===========================================================================*/



/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "sapi.h"
#include "keys.h"
#include "tarea.h"
#include "task.h"

/*=====[Definicionde de datos internos]==============================*/
const gpioMap_t leds_t[] = {LEDG,LED1,LED2};
const gpioMap_t gpio_t[] = {GPIO7,GPIO5,GPIO3};
/*=====[Definitions of extern global functions]==============================*/
extern t_key_config* keys_config;

#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
// Prototipo de funcion de la tarea
void task_led( void* taskParmPtr );
void gpio_init( void );

/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
    BaseType_t res;

    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();  // Inicializar y configurar la plataforma

    gpio_init();

    printf( "Ejercicio Practica Parcial\n" );

    // Crear tareas en freeRTOS
    for ( uint32_t i = 0 ; i < LED_COUNT ; i++ ){
    	res = xTaskCreate (
              task_led,					// Funcion de la tarea a ejecutar
              ( const char * )"task_led",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
              i,							// Parametros de tarea
              tskIDLE_PRIORITY+1,			// Prioridad de la tarea
              0							// Puntero a la tarea creada en el sistema
          );

    // Gestión de errores
    configASSERT( res == pdPASS );
    }

	res = xTaskCreate (
		  tarea,					// Funcion de la tarea a ejecutar
		  ( const char * )"tarea",	// Nombre de la tarea como String amigable para el usuario
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

void gpio_init(void)
{
	for ( uint32_t i = 0 ; i < LED_COUNT ; i++ ){
		gpioInit ( gpio_t[i], GPIO_OUTPUT );
	}
}

void task_led( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;

    TickType_t dif =   pdMS_TO_TICKS( 500 );

    TickType_t xPeriodicity = pdMS_TO_TICKS( 1000 ); // Tarea periodica cada 1000 ms

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while( 1 )
    {
        /* consulto de manera no bloqueante si se pulso una tecla */

    	if( key_pressed( index ) )
        {
            dif = keys_get_diff( index );
        }

        gpioWrite( leds_t[index], ON );
        gpioWrite( gpio_t[index], ON );
        vTaskDelay( dif );
        gpioWrite( leds_t[index], OFF );
        gpioWrite( gpio_t[index], OFF );

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
