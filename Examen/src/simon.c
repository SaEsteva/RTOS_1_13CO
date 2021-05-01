#include "simon.h"
#include "random.h"
#include "sapi.h"
#include "keys.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define SIMON_MAX_ROUNDS            5
#define SIMON_BLINK_PAUSE           300
#define SIMON_SEQUENCE_PAUSE        300
#define SIMON_USER_INPUT_TIMEOUT    2000
#define N_QUEUE 	3

#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error creating the queue.\r\n"
#define LED_ERROR LEDR

typedef enum{
	GAME_OVER,
	NEW_ROUND,
	NEW_TEC,
	WIN
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
	xQueueHandle queue_user; //almacenara el evento en una cola
	game_status status;
	bool_t new_round;
	bool_t data_to_send;
} t_simon;

typedef struct
{
	gpioMap_t 	tec_touch;
	game_status status;
	int 		round;
    TickType_t 	time_play;	//variables
} tPrint;

QueueHandle_t queue_sec_ready;

t_simon simon_data;

t_led leds_data[] = { {.led= LEDB}, {.led= LED1}, {.led= LED2}, {.led= LED3}};
#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])

void simon_service_user( void* pvParameters );
void simon_service_logic( void* pvParameters );
void SendUart( tPrint *  );


/* INICIALIZACION DE RECURSOS DEL JUEGO */
void simon_init()
{


	BaseType_t res;

	res = xTaskCreate (
			simon_service_logic,					// Funcion de la tarea a ejecutar
		  ( const char * )"simon_service_logic",	// Nombre de la tarea como String amigable para el usuario
		  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
		  0,							// Parametros de tarea
		  tskIDLE_PRIORITY+4,			// Prioridad de la tarea
		  0							// Puntero a la tarea creada en el sistema
	  );

	// Gestion de errores
	configASSERT( res == pdPASS );

	res = xTaskCreate (
			simon_service_user,					// Funcion de la tarea a ejecutar
		  ( const char * )"simon_service_user",	// Nombre de la tarea como String amigable para el usuario
		  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
		  0,							// Parametros de tarea
		  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
		  0							// Puntero a la tarea creada en el sistema
	  );

	// Gestion de errores
	configASSERT( res == pdPASS );

	queue_sec_ready = xQueueCreate( 1, sizeof( int ) );

	// Gestion de errores de colas
	if( queue_sec_ready == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_QUE );
		while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}


}

/* usado por la tarea simon_service_logic para encender secuencias */
void simon_pulse_led( uint32_t idx )
{
	gpioWrite( leds_data[idx].led, ON );
	vTaskDelay( SIMON_BLINK_PAUSE );
	gpioWrite( leds_data[idx].led,OFF );
	vTaskDelay( SIMON_SEQUENCE_PAUSE*2 );
}

/* usado por la tarea simon_service_user para confirmar pulsaciones */
void simon_pulse_led_user( gpioMap_t led )
{
	gpioWrite( led, ON );
	vTaskDelay( SIMON_BLINK_PAUSE );
	gpioWrite(  led,OFF );
}

void simon_service_logic( void* pvParameters )
{
	int game_alive;
	int round;
	char leds_rounds[SIMON_MAX_ROUNDS];
	BaseType_t result;

	tPrint print_data;

	game_alive = 0;
	t_key_isr_signal evnt;

	t_led led_sec,led_send;
	t_simon* data_logic;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	data_logic = &simon_data;
	data_logic->queue_user = xQueueCreate( SIMON_MAX_ROUNDS, sizeof( t_led ) ); //creo la cola que recibe esto
	data_logic->queue_game = xQueueCreate( SIMON_MAX_ROUNDS, sizeof( t_led ) ); //creo la cola que recibe esto

	if( data_logic->queue_user == NULL || data_logic->queue_game == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_QUE );
		while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

	data_logic->new_round = 1;

	while( 1 )
	{
		if (!game_alive){
			xQueueReceive( data_logic->queue_user, &led_sec, portMAX_DELAY );
			xQueueReset(data_logic->queue_user);
			xQueueReset(data_logic->queue_game);
			game_alive = 1;
		}

		if (data_logic->new_round){
			data_logic->Nround++;
			for (round = 0 ; round < data_logic->Nround; round++ ){
				led_sec = leds_data[1];
				xQueueSend( data_logic->queue_game, &led_sec,  portMAX_DELAY );
				simon_pulse_led(1);
			}
			data_logic->time_game = xTaskGetTickCount();
			round = 0;
			data_logic->new_round = 0;
		}


       	xQueueSend( queue_sec_ready, &game_alive,  portMAX_DELAY );

       	result = xQueueReceive( data_logic->queue_user, &led_send,  0 );
		if (result == pdTRUE){
			xQueueReceive( data_logic->queue_game, &led_sec,  0 );
			if (led_sec.led == led_send.led){
					round++;
					if (round >= data_logic->Nround){
						data_logic->new_round;
						data_logic->data_to_send = 1;
						data_logic->status = NEW_ROUND;
						data_logic->Nround++;
					}
			}else {
					data_logic->data_to_send = 1;
					data_logic->status = GAME_OVER;
			}
		}

		xLastWakeTime = xTaskGetTickCount() - data_logic->time_game;
		if (xLastWakeTime >= SIMON_USER_INPUT_TIMEOUT){
			data_logic->data_to_send = 1;
			data_logic->status = GAME_OVER;
		}

		if (data_logic->new_round || data_logic->Nround > SIMON_MAX_ROUNDS){
			data_logic->data_to_send = 1;
			data_logic->status = WIN;
		}
		if (data_logic->data_to_send){
			print_data.status=data_logic->status;
			print_data.round = data_logic->new_round;
			print_data.time_play = xTaskGetTickCount() - data_logic->time_game;
			SendUart(&print_data);
			data_logic->data_to_send = 0;
		}
	}
}


void simon_service_user( void* pvParameters )
{
	int game_alive;
	tPrint print_data;

	t_led user_led;

	t_simon* data_user;

	data_user = &simon_data;

	game_alive = 0;

	while( 1 )
	{
		if (!game_alive)
			xQueueReceive( queue_sec_ready, &game_alive,  portMAX_DELAY );

		xQueueSend( data_user->queue_user, &user_led,  portMAX_DELAY );

		simon_pulse_led_user(user_led.led);

		print_data.status=NEW_TEC;
		print_data.tec_touch=user_led.led;
		SendUart(&print_data);
		game_alive = 0;
	}
}


void SendUart( tPrint * datos )
{
          switch(datos->status){
        	case GAME_OVER:
        		taskENTER_CRITICAL();
        		printf( "GAME OVER ");
    		    taskEXIT_CRITICAL();
				break;
        	case NEW_ROUND:
            	taskENTER_CRITICAL();
        		printf( "Go to round %d\r\n",datos->round);
        		taskEXIT_CRITICAL();
            	break;
        	case NEW_TEC:
				taskENTER_CRITICAL();
				printf( "You push the tec %d \r\n",datos->tec_touch-TEC1+1);
				taskEXIT_CRITICAL();
				break;
        	case WIN:
				taskENTER_CRITICAL();
				printf( "You win the %d rounds of the game\r\n",datos->round);
				taskEXIT_CRITICAL();
				break;
        	default:
            	taskENTER_CRITICAL();
        		printf( "Error, incorrect status");
        		taskEXIT_CRITICAL();
        		break;
        }
}


void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data )
{
    //nada
}

void user_keys_event_handler_button_release( t_key_isr_signal* event_data )
{
	t_simon* data_handler;

	data_handler = &simon_data;

	// agrego a la cola de usuario los valores que recibe la interrupcion
    if( event_data->tecla == TEC1_INDEX )
    {
    	xQueueSend( data_handler->queue_user, &leds_data[TEC1_INDEX],portMAX_DELAY );
    }

    if( event_data->tecla == TEC2_INDEX )
    {
        xQueueSend( data_handler->queue_user, &leds_data[TEC2_INDEX], portMAX_DELAY );
    }

    if( event_data->tecla == TEC3_INDEX )
    {
        xQueueSend( data_handler->queue_user, &leds_data[TEC3_INDEX], portMAX_DELAY );
    }

    if( event_data->tecla == TEC4_INDEX )
    {
        xQueueSend( data_handler->queue_user, &leds_data[TEC4_INDEX], portMAX_DELAY );
    }
}
