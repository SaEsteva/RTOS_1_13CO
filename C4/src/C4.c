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
#include "semphr.h"

#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)

<<<<<<< HEAD
#define WELCOME_MSG  "Ejercicio C_4.\r\n"
=======
#define WELCOME_MSG  "Ejercicio C_2.\r\n"
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_SEM "Error al crear los semaforos.\r\n"
<<<<<<< HEAD
#define LED_ERROR LED1
/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDR,LEDG};
gpioMap_t gpio_t[] = {GPIO7,GPIO5};
SemaphoreHandle_t sem_btn;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init(void);
=======
#define LED_ERROR LEDR
/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB,LED1,LED2,LED3};
gpioMap_t gpio_t[] = {GPIO7,GPIO5,GPIO3,GPIO1};
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config keys_config[];

#define LED_COUNT   sizeof(leds_t)/sizeof(leds_t[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );
<<<<<<< HEAD

=======
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
<<<<<<< HEAD
	boardConfig();									// Inicializar y configurar la plataforma

	gpio_init();

	debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
	printf( WELCOME_MSG );

	BaseType_t res;
	uint32_t i;

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
	
=======
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
    printf( WELCOME_MSG );

    BaseType_t res;
    uint32_t i;

    // Crear tarea en freeRTOS
    for ( i = 0 ; i < LED_COUNT/2 ; i++ )
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
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe

    // Inicializo driver de teclas
    keys_Init();

<<<<<<< HEAD
    // Crear semaforo
    sem_btn = xSemaphoreCreateBinary(  );

    // Gestion de errores de semaforos
	if( sem_btn == NULL)
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_SEM );
		while(TRUE);						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

=======
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
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
<<<<<<< HEAD
void gpio_init(void)
{
	uint32_t i;

	for( i = 0 ; i < LED_COUNT; i++)
	{
		gpioInit (gpio_t[i], GPIO_OUTPUT);
	}
=======
void gpio_init( void )
{
    uint32_t i;

    for( i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{
<<<<<<< HEAD
	uint32_t index = 0;

    // ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
	TickType_t xLastWakeTime = xTaskGetTickCount();
	TickType_t Wait = LED_RATE*3;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
    	 
		gpioWrite( leds_t[index], ON );
		gpioWrite( gpio_t[index] , ON );
		vTaskDelay( xPeriodicity / 4 );
		gpioWrite( leds_t[index], OFF );
		gpioWrite( gpio_t[index] , OFF );
       
      if ( xSemaphoreTake( sem_btn , Wait/4 ) == pdTRUE ){    // Esperamos tecla durante 3/4 segundo si la recibe toma el led verde
         index = 1;
      }
      else{ //no recibio tecla durante el tiempo, enciende led rojo
         index = 0;
      }
       
      vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
      //
=======
    uint32_t index = ( uint32_t ) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif;
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        if ( xSemaphoreTake( keys_config[index].sem_btn, xPeriodicity ) == pdTRUE )	//esperar tecla
        {
            // llego semaforo
            keys_config[index].led = ( keys_config[index].btn == TEC1 )?( leds_t[0] ):( leds_t[2] );
            keys_config[index].gpio = ( keys_config[index].btn == TEC1 )?( gpio_t[0] ):( gpio_t[2] );
        }
        else
        {
            keys_config[index].led = ( keys_config[index].btn == TEC1 )?( leds_t[1] ):( leds_t[3] );
            keys_config[index].gpio = ( keys_config[index].btn == TEC1 )?( gpio_t[1] ):( gpio_t[3] );
        }

        gpioWrite( keys_config[index].led, ON );
        gpioWrite( keys_config[index].gpio, ON );
        vTaskDelay( LED_RATE / 2 );
        gpioWrite( keys_config[index].led, OFF );
        gpioWrite( keys_config[index].gpio, OFF );

        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
<<<<<<< HEAD
	printf( MALLOC_ERROR );
	configASSERT( 0 );
=======
    printf( MALLOC_ERROR );
    configASSERT( 0 );
>>>>>>> f7f20ad8c8d6a0a62ebe77df343f487224685ffe
}
/*==================[fin del archivo]========================================*/
