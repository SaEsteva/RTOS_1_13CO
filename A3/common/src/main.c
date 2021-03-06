/* Copyright 2020, Franco Bucafusco
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "config.h"
#include "task.h"
#include <stdio.h>
/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

print_t debugPrint;

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/
TaskHandle_t task_handle_a;
TaskHandle_t task_handle_b;
TaskHandle_t task_handle_c;
TaskHandle_t task_handle_d;

// Prototipo de funcion de la tarea
void tarea_A_code( void*  );
void tarea_B_code( void*  );
void tarea_C_code( void*  );
void tarea_D_code( void*  );


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
	BaseType_t res;

	// ---------- CONFIGURACIONES ------------------------------
	PRINTF_CONFIGURE;
	PRINTF( EXAMPLE_WELCOME_TEXT );

	/* solo creo la tarea A */
	res = xTaskCreate(
			  tarea_A_code,               // Funcion de la tarea a ejecutar
			  ( const char * )"tarea_a",  // Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2, /* tamaño del stack de cada tarea (words) */
			  NULL,                       // Parametros de tarea
			  tskIDLE_PRIORITY+4,         // Prioridad de la tarea
			  &task_handle_a            // Referencia a la tarea creada en el sistema
		  );

	configASSERT( res == pdPASS );


	// Iniciar scheduler
	vTaskStartScheduler();

	// ---------- REPETIR POR SIEMPRE --------------------------
	while( 1 )
	{
		// Si cae en este while 1 significa que no pudo iniciar el scheduler
	}

	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.
	return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/**
   @brief   Implementacion de un delay utilizando ciclos for.
			Se implementa solo para generar utilizacion de CPU sin bloquear a la tarea.

			NO DEBE UTILIZARSE BAJO NINGUN PUNTO DE VISTA EN UN APLICACION REAL SOBRE UN RTOS.

   @param cuentas
 */
void delay_con_for( uint32_t ms )
{
	volatile uint32_t i;
	volatile uint32_t j;

	for( i=0 ; i<ms ; i++ )
	{
		/* delay de aprox 1 ms */
		for( j=0 ; j<CUENTAS_1MS ; j++ )
		{
		}
	}
}

void blink_n_500( uint32_t n, uint32_t led )
{
	/* genero 2 blinks*/
	int blink_count  = n;
	int cycles       = blink_count*2;

	for( ; cycles>0 ; cycles-- )
	{
		gpioToggle( led );
		delay_con_for( 500 );
	}
}

/*==================[definiciones de funciones externas]=====================*/
void tarea_A_code( void* taskParmPtr )
{
	BaseType_t res;

	PRINTF( "Tarea A\r\n" );

	UBaseType_t my_prio = uxTaskPriorityGet( 0 );   /* se podria haber usado uxTaskPriorityGet( task_handles[0] ) */

    /* creo la tarea B con mnor prioridad que la que esta corriendo actualmente
       No hay CC */
	res = xTaskCreate(
			  tarea_B_code,                // Funcion de la tarea a ejecutar
			  ( const char * )"tarea_b",   // Nombre de fantasia
			  configMINIMAL_STACK_SIZE*2,  /* tamaño del stack de cada tarea (words) */
			  NULL,                        // Parametros de tarea
			  my_prio - 1,                 /* le doy menos prioridad que la tarea actual (la A) */
			  &task_handle_b               // Referencia a la tarea creada en el sistema
		  );

	configASSERT( res == pdPASS );

    /* creo la tarea C con mnor prioridad que la que esta corriendo actualmente
       No hay CC */
	res = xTaskCreate(
			  tarea_C_code,                // Funcion de la tarea a ejecutar
			  ( const char * )"tarea_c",   // Nombre de fantasia
			  configMINIMAL_STACK_SIZE*2,  /* tamaño del stack de cada tarea (words) */
			  NULL,                        // Parametros de tarea
			  my_prio - 2,                 /* le doy menos prioridad que la tarea actual (la A) */
			  &task_handle_c               // Referencia a la tarea creada en el sistema
		  );

	configASSERT( res == pdPASS );

    /* creo la tarea D con mnor prioridad que la que esta corriendo actualmente
       No hay CC */
	res = xTaskCreate(
			  tarea_D_code,                // Funcion de la tarea a ejecutar
			  ( const char * )"tarea_d",   // Nombre de fantasia
			  configMINIMAL_STACK_SIZE*2,  /* tamaño del stack de cada tarea (words) */
			  NULL,                        // Parametros de tarea
			  my_prio - 3,                 /* le doy menos prioridad que la tarea actual (la A) */
			  &task_handle_d               // Referencia a la tarea creada en el sistema
		  );

	configASSERT( res == pdPASS );

	/* suspendo la tarea actual */
	vTaskSuspend( 0 );                              /* se podria haber usado vTaskSuspend( task_handles[0] ) */

	blink_n_500( 2 , LEDB );

	/* suspendo la tarea actual */
	vTaskSuspend( task_handle_a );                /* se podria haber usado vTaskSuspend( 0 ) */
}

void tarea_B_code( void* taskParmPtr )
{
	PRINTF( "Tarea B\r\n" );

	blink_n_500( 3, LED1 );

	/* retomo tarea A, como tiene mayor prioridad, habra un cambio de contexto.*/
	vTaskResume( task_handle_a );

	/* suspendo la tarea actual, le va a dar el CPU a las tareas de menor prioridad (porque la tarea A esta suspendida) */
	vTaskSuspend( task_handle_b );   /* se podria haber usado vTaskSuspend( 0 ) */
}

void tarea_C_code( void* taskParmPtr )
{
	PRINTF( "Tarea C\r\n" );

	blink_n_500( 3, LED2 );

	/* suspendo la tarea actual, le va a dar el CPU a las tareas de menor prioridad (porque la tarea A esta suspendida) */
	vTaskSuspend( task_handle_c);       /* se podria haber usado vTaskSuspend( 0 ) */
}

void tarea_D_code( void* taskParmPtr )
{
	PRINTF( "Tarea D\r\n" );

	/* termino todas las tareas de mas prioridad */
	vTaskDelete( task_handle_a );
	vTaskDelete( task_handle_b );
	vTaskDelete( task_handle_c );

	while( 1 )
	{
		gpioToggle( LED3 );
		delay_con_for( 500 );
	}
}

/*==================[fin del archivo]========================================*/
