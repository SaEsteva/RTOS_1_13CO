/*=============================================================================
 * Copyright (c) 2021, Joaquin Gaspar Ulloa <joaquin.g.ulloa@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/04
 * Version: v1.0
 *===========================================================================*/

#include "../inc/pwm.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"

#include "print.h"
#include "keys.h"
#include "random.h"

typedef enum{
	START_GAME,
	NEW_ROUND,
	NEW_TEC,
	FINISH_ROUND,
	GAME_OVER,
	WIN,
	GAME_ERROR
}game_status;

typedef enum{
	MODE_UP,
	MODE_DOWN,
	MODE_TRIANGLE
} pwm_mode_t;

typedef struct
{
	gpioMap_t led;
} t_led;

typedef struct
{
	xQueueHandle queue;
} t_pwm_tec;

typedef struct
{
	xQueueHandle 	queue;
} t_pwm;

typedef struct
{
	TickType_t 	period;
	pwm_mode_t 	mode;
	bool_t		on;
} t_pwm_status;

typedef struct
{
	int 	tec_touch;
	game_status status;
	int 		round;
    TickType_t 	time_play;	//variables
} tPrint;

SemaphoreHandle_t sem_sec_ready;

// Estructura con todos los datos
t_pwm_tec 	pwm_data_tec;
t_pwm		pwm_data;

t_led leds_data[] = { {.led= LEDB}, {.led= LED1}, {.led= LED2}, {.led= LED3}};
#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])

void pwm_pulse_led( uint32_t idx );
void pwm_leds( void* pvParameters );
void pwm_tec( void* pvParameters );
void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data );
void user_keys_event_handler_button_release( t_key_isr_signal* event_data );

/* INICIALIZACION DE RECURSOS DEL JUEGO */
void pwm_init()
{

	BaseType_t res;

	// Creacion de tareas
	res = xTaskCreate (
			pwm_leds,					// Funcion de la tarea a ejecutar
		  ( const char * )"pwm_leds",	// Nombre de la tarea como String amigable para el usuario
		  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
		  0,							// Parametros de tarea
		  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
		  0								// Puntero a la tarea creada en el sistema
	  );

	// Gestion de errores
	configASSERT( res == pdPASS );

	res = xTaskCreate (
			pwm_tec,					// Funcion de la tarea a ejecutar
		  ( const char * )"pwm_tec",	// Nombre de la tarea como String amigable para el usuario
		  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
		  0,							// Parametros de tarea
		  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
		  0								// Puntero a la tarea creada en el sistema
	  );

	// Gestion de errores de tareas
	configASSERT( res == pdPASS );

	// Creacion de colas
	pwm_data_tec.queue = xQueueCreate( 10, sizeof( int8_t ) ); //creo la cola que recibe los datos de laas teclas
	pwm_data.queue = xQueueCreate( 10, sizeof( t_pwm_status ) ); //creo la cola que envia el dato desde tarea 2 a 1

	// Gestion de errores de colas
	if( pwm_data_tec.queue == NULL || pwm_data.queue == NULL){
		gpioWrite( LEDR, ON );
		printf( MSG_ERROR_QUE );
		while( TRUE );
	}

	// Creacion de semaforos
	sem_sec_ready = xSemaphoreCreateBinary();

	// Gestion de errores de semaforos
	if( sem_sec_ready == NULL )
	{
		gpioWrite( LEDR, ON );
		printf( MSG_ERROR_SEM );
		while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

}

/* usado por la tarea pwm_leds para encender secuencias */
void pwm_pulse_led( uint32_t idx )
{
	gpioWrite( leds_data[idx].led, ON );
	vTaskDelay( SIMON_BLINK_PAUSE );
	gpioWrite( leds_data[idx].led,OFF );
	vTaskDelay( SIMON_SEQUENCE_PAUSE*2 );
}

void pwm_leds( void* pvParameters )
{
	BaseType_t received;
	t_pwm_status status;
	TickType_t led_step;
	TickType_t duty_cycle;
	TickType_t duty_cycle_n;
	// debe almecenar el porcentaje
	while( 1 ){
		// Lee los datos de la otra tarea
		xQueueReceive( pwm_data.queue, &status, portMAX_DELAY );
		//received = xQueueReceive( pwm_data.queue, &status, 0 );
		// DEBUG
		gpioWrite( LED3, ON );
		vTaskDelay( SIMON_BLINK_PAUSE );
		gpioWrite( LED3,OFF );
		vTaskDelay( SIMON_SEQUENCE_PAUSE*2 );
		// Actualiza las variables locales
		//if(pdTRUE == xQueueReceive( pwm_data.queue, &status, 0 )){
			led_step = status.period / 10;
		//}
		// chequea si está en on
		if(status.on != TRUE){
			// Se apaga el led
			gpioWrite( LED1, OFF );
		}
		else{
			switch (status.mode){
				case MODE_UP:
					//led_step = status.period / 10;
					duty_cycle = duty_cycle + led_step;
					duty_cycle_n = status.period - duty_cycle;
					if(duty_cycle > status.period){
						duty_cycle = 1000;
					}
					gpioWrite( LED1, ON );
					vTaskDelay( duty_cycle );
					gpioWrite( LED1, OFF );
					vTaskDelay( duty_cycle_n );
					break;
				case MODE_DOWN:
					duty_cycle = duty_cycle + led_step;
					if(duty_cycle < 1000){
						duty_cycle = status.period;
					}
					gpioWrite( LED1, ON );
					vTaskDelay( duty_cycle );
					gpioWrite( LED1,OFF );
					vTaskDelay( duty_cycle_n );
					break;
				case MODE_TRIANGLE:
					// DEBUG
					gpioWrite( LED2, ON );
					vTaskDelay( SIMON_BLINK_PAUSE );
					gpioWrite( LED2,OFF );
					vTaskDelay( SIMON_SEQUENCE_PAUSE*2 );
					break;
			}
		}
	}
}


void pwm_tec( void* pvParameters )
{
	int8_t tecla;
	TickType_t dif = pdMS_TO_TICKS( 0 );
	int period = 1000;
	pwm_mode_t pwm_mode = MODE_UP;
	bool_t pwn_on = TRUE;
	t_pwm_status status;
	t_pwm_status status2;

	while( 1 ){
		// lee la tecla
		xQueueReceive( pwm_data_tec.queue, &tecla,  portMAX_DELAY );
		// veo si es 1, 2 o 3
		switch (tecla){
			case TEC1_INDEX:
				dif = keys_get_diff( TEC1_INDEX );
				if(dif <= pdMS_TO_TICKS( 500 )){ // cambiar a generic
					// Se cambia de modo de manera incremental
					pwm_mode++;
					if (pwm_mode > MODE_TRIANGLE){
						pwm_mode = MODE_UP;
					}
				}
				else{
					// Se prende o paga
					pwn_on =! pwn_on;
				}
				break;
			case TEC2_INDEX:
				// 2 T++ mas t=2s min t=1s
				if (period < 2000){
					period = period + 100;
				}
				else{
					period = 2000;
				}
				break;
			case TEC3_INDEX:
				// 3 T-- mas t=2s min t=1s
				if (period > 1000){
					period = period - 100;
				}
				else{
					period = 1000;
				}
				break;
		}
		// Acondicionar datos
		status.mode = pwm_mode;
		status.on = pwn_on;
		status.period = pdMS_TO_TICKS( period );
		// Se envian los datos
		xQueueSend( pwm_data.queue, &status, portMAX_DELAY );
	}
}

void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data )
{
    //nada
}

void user_keys_event_handler_button_release( t_key_isr_signal* event_data )
{
	int8_t tecla;

	if( event_data->tecla == TEC1_INDEX ){
		tecla = TEC1_INDEX;
		xQueueSend( pwm_data_tec.queue, &tecla, portMAX_DELAY );
	}
	if( event_data->tecla == TEC2_INDEX ){
		tecla = TEC2_INDEX;
		xQueueSend( pwm_data_tec.queue, &tecla, portMAX_DELAY );
	}
	if( event_data->tecla == TEC3_INDEX ){
		tecla = TEC3_INDEX;
		xQueueSend( pwm_data_tec.queue, &tecla, portMAX_DELAY );
	}
	/*if( event_data->tecla == TEC4_INDEX ){
		xQueueSend( pwm_data_tec.queue, &event_data, portMAX_DELAY );
	}*/
}
