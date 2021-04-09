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
#include "semphr.h"

/*==================[definiciones y macros]==================================*/
#define EVIDENCIAR_PROBLEMA 1


typedef struct
{
    char* nombre;
    uint32_t delay;
} t_task_param;
/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/
const t_task_param params[4]=
{
    { "tarea 1", 33},
    { "tarea 2", 55},
    { "tarea 3", 77},
    { "tarea 4", 99},
};

SemaphoreHandle_t mutex;

/*==================[declaraciones de funciones externas]====================*/
// Prototipo de funcion de la tarea
void task( void* taskParmPtr );

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
              task,                  // Funcion de la tarea a ejecutar
              ( const char * )"t1",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              &params[0],                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                             // Puntero a la tarea creada en el sistema
          );

    res = xTaskCreate(
              task,                  // Funcion de la tarea a ejecutar
              ( const char * )"t2",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              &params[1],                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                             // Puntero a la tarea creada en el sistema
          );

    res = xTaskCreate(
              task,                  // Funcion de la tarea a ejecutar
              ( const char * )"t3",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              &params[2],                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                             // Puntero a la tarea creada en el sistema
          );

    res = xTaskCreate(
              task,                  // Funcion de la tarea a ejecutar
              ( const char * )"t4",   	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
              &params[3],                          	// Parametros de tarea
              tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
              0                             // Puntero a la tarea creada en el sistema
          );

    configASSERT( res == pdPASS ); // gestion de errores

    mutex = xSemaphoreCreateBinary();
    xSemaphoreGive( mutex );

    // Iniciar scheduler
    vTaskStartScheduler(); // Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void task( void* taskParmPtr )
{
    t_task_param* param = ( t_task_param* ) taskParmPtr;

    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        /* abro seccion critica */
        xSemaphoreTake( mutex, portMAX_DELAY );

        printf( "Hola soy la tarea %s\n", param->nombre );

        /* cierro seccio ncritica */
        xSemaphoreGive( mutex );

        vTaskDelay( param->delay / portTICK_RATE_MS );
    }
}


/*==================[fin del archivo]========================================*/
