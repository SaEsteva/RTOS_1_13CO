/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// sAPI header
#include "sapi.h"
#include "FreeRTOSConfig.h"

/*==================[definiciones y macros]==================================*/
#define EVIDENCIAR_PROBLEMA 1

#if EVIDENCIAR_PROBLEMA==1
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START
#define CRITICAL_END
#else
#include "semphr.h"
#define CRITICAL_DECLARE    SemaphoreHandle_t mutex
#define CRITICAL_CONFIG     mutex = xSemaphoreCreateMutex()
#define CRITICAL_START      xSemaphoreTake( mutex , portMAX_DELAY )
#define CRITICAL_END        xSemaphoreGive( mutex )
#endif

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/
CRITICAL_DECLARE;

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void sacerdote_a( void* taskParmPtr );
void sacerdote_b( void* taskParmPtr );
void sacerdote_c( void* taskParmPtr );
void sacerdote_d( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    BaseType_t res;

    // ---------- CONFIGURACIONES ------------------------------
    // Inicializar y configurar la plataforma
    boardConfig();

    // UART for debug messages
    debugPrintConfigUart( UART_USB, 9600 );

    printf( "Ejercicio printf.\n" );

    // Crear tarea en freeRTOS
    res = xTaskCreate(
              sacerdote_a,                  // Funcion de la tarea a ejecutar
              ( const char * )"myTask",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              0,                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                             // Puntero a la tarea creada en el sistema
          );

    configASSERT( res == pdPASS ); // gestion de errores

    res = xTaskCreate(
              sacerdote_b,                  // Funcion de la tarea a ejecutar
              ( const char * )"myTask",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              0,                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                          	// Puntero a la tarea creada en el sistema
          );

    configASSERT( res == pdPASS ); // gestion de errores


    res = xTaskCreate(
              sacerdote_c,                  // Funcion de la tarea a ejecutar
              ( const char * )"myTask",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              0,                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                       		// Puntero a la tarea creada en el sistema
          );

    configASSERT( res == pdPASS ); // gestion de errores

    res = xTaskCreate(
              sacerdote_d,                  // Funcion de la tarea a ejecutar
              ( const char * )"myTask",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              0,                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                        		// Puntero a la tarea creada en el sistema
          );

    configASSERT( res == pdPASS ); // gestion de errores

    CRITICAL_CONFIG;

    // Iniciar scheduler
    vTaskStartScheduler(); // Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void sacerdote_a( void* taskParmPtr )
{
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        CRITICAL_START; //abrir seccion critica
        printf( "El aldeano es ROJO \n" );
        CRITICAL_END;	//cerrar seccion critica

        vTaskDelay( 99 / portTICK_RATE_MS );
    }
}

void sacerdote_b( void* taskParmPtr )
{
    while( TRUE )
    {
        CRITICAL_START; //abrir seccion critica
        printf( "El aldeano es AZUL \n" );
        CRITICAL_END;	//cerrar seccion critica

        vTaskDelay( 77 / portTICK_RATE_MS );
    }
}

void sacerdote_c( void* taskParmPtr )
{
    while( TRUE )
    {
        CRITICAL_START; //abrir seccion critica
        printf( "El aldeano es GRIS \n" );
        CRITICAL_END;	//cerrar seccion critica

        vTaskDelay( 55 / portTICK_RATE_MS );
    }
}

void sacerdote_d( void* taskParmPtr )
{
    while( TRUE )
    {
        CRITICAL_START; //abrir seccion critica
        printf( "El aldeano es CIAN \n" );
        CRITICAL_END;	//cerrar seccion critica

        vTaskDelay( 33 / portTICK_RATE_MS );
    }
}

/*==================[fin del archivo]========================================*/
