#ifndef SIMON_H
#define SIMON_H

#include "random.h"
#include "sapi.h"
#include "keys.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define SIMON_LED_ID_GAME_OVER 		-1
#define SIMON_MAX_ROUNDS            5 // If this number is more than 10, resize ISR_QUEUE_SIZE in keys.h
#define SIMON_BLINK_PAUSE           300
#define SIMON_SEQUENCE_PAUSE        300
#define SIMON_USER_INPUT_TIMEOUT    50000
#define N_QUEUE 	3

#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error creating the queue.\r\n"
#define MSG_ERROR_SEM "Error creating the semafore.\r\n"
#define LED_ERROR LEDR

typedef enum{
	START_GAME,
	NEW_ROUND,
	NEXT_ROUND,
	NEW_TEC,
	FINISH_ROUND,
	GAME_OVER,
	WIN,
	GAME_ERROR
}game_status;

typedef struct
{
	gpioMap_t led;
} t_led;

typedef struct
{
	TickType_t 	time_game;
	int 		Nround;
	xQueueHandle queue_game; //almacenara el evento en una cola
	game_status status;
} t_simon;

typedef struct
{
	int 	tec_touch;
	game_status status;
	int 		round;
    TickType_t 	time_play;	//variables
} tPrint;


void simon_init();
void simon_pulse_led( uint32_t idx );
void simon_pulse_led_user( uint32_t idx );
void inicializarSecuencia(uint32_t *secuencia, uint32_t maxValues);
void init_game_data(uint32_t first_init);

void simon_service_logic( void* pvParameters );

void simon_service_user( void* pvParameters );
void SendUart( tPrint * datos );
void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data );
void user_keys_event_handler_button_release( t_key_isr_signal* event_data );

#endif
