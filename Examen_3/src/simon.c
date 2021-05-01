#include "simon.h"



QueueHandle_t queue_tec_touch;

SemaphoreHandle_t sem_sec_ready;

// Estructura con todos los datos
t_simon simon_data;


t_led leds_data[] = { {.led= LEDB}, {.led= LED1}, {.led= LED2}, {.led= LED3}};
#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])


void inicializarSecuencia(uint32_t *secuencia, uint32_t maxValues){
	random_seed_freertos();
	for(uint8_t i = 0; i<maxValues; i++){
		secuencia[i] = random(0, maxValues-1);
	}
}

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

	sem_sec_ready = xSemaphoreCreateBinary();

	// Gestion de errores de semaforos
	if( sem_sec_ready == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_SEM );
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
void simon_pulse_led_user( uint32_t idx )
{
	gpioWrite( leds_data[idx].led, ON );
	vTaskDelay( SIMON_BLINK_PAUSE );
	gpioWrite( leds_data[idx].led,OFF );
	vTaskDelay( SIMON_SEQUENCE_PAUSE*2 );
}

void init_game_data(uint32_t first_init){

	simon_data.Nround = 1;
	simon_data.status = START_GAME;
	simon_data.time_game = xTaskGetTickCount();

	if (first_init){
		simon_data.queue_game = xQueueCreate( SIMON_MAX_ROUNDS, sizeof( int ) ); //creo la cola que recibe los datos de laas teclas
		queue_tec_touch = xQueueCreate( 1, sizeof( int ) ); //creo la cola que envia el dato desde tarea 2 a 1

		if( simon_data.queue_game == NULL || queue_tec_touch == NULL)
		{
			gpioWrite( LED_ERROR, ON );
			printf( MSG_ERROR_QUE );
			while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
		}
	}


}

void simon_service_logic( void* pvParameters )
{
	int game_alive = 0;
	int round;
	int leds_rounds[SIMON_MAX_ROUNDS];

	tPrint print_data;

	int led_send;

	init_game_data(1);


	while( 1 )
	{

		if (!game_alive){
			xQueueReceive( simon_data.queue_game, &led_send, portMAX_DELAY );
			simon_pulse_led(led_send);
			init_game_data(0);
			game_alive = 1;
		}

		print_data.status = simon_data.status;
		print_data.round = simon_data.Nround;
		print_data.time_play = xTaskGetTickCount() - simon_data.time_game;
		print_data.tec_touch = led_send;
		SendUart(&print_data);

		switch (simon_data.status){
			case START_GAME:
				xQueueReset(simon_data.queue_game);
				simon_data.status = NEW_ROUND;
				break;
			case NEW_ROUND:
				inicializarSecuencia((uint32_t *)(&leds_rounds[0]), SIMON_MAX_ROUNDS);
				simon_pulse_led(leds_rounds[0]);
				simon_data.time_game = xTaskGetTickCount();
				round = 0;
				simon_data.status = NEW_TEC;
				xSemaphoreGive( sem_sec_ready );
				break;
			case NEXT_ROUND:
				for (round = 0 ; round < simon_data.Nround; round++ ){
					simon_pulse_led(leds_rounds[round]);
				}
				round = 0;
				simon_data.status = NEW_TEC;
				xSemaphoreGive( sem_sec_ready );
				break;
			case NEW_TEC:
				xQueueReceive( queue_tec_touch, &led_send,  portMAX_DELAY );
				if (led_send == leds_rounds[round]){
						round++;
						if (round >= simon_data.Nround){
							simon_data.status = FINISH_ROUND;
							simon_data.Nround++;
						}
				}else {
					simon_data.status = GAME_OVER;
				}
				break;
			case FINISH_ROUND:
				if (simon_data.Nround > SIMON_MAX_ROUNDS){
					simon_data.status = WIN;
				}else{
					simon_data.status = NEXT_ROUND;
				}
				break;
			case GAME_OVER:
				print_data.status = simon_data.status;
				SendUart(&print_data);
				game_alive = 0;
				break;
			case WIN:
				print_data.status = simon_data.status;
				SendUart(&print_data);
				game_alive = 0;
				break;
			default:
				print_data.status = GAME_ERROR;
				SendUart(&print_data);
				game_alive = 0;
				break;
		}
	}
}


void simon_service_user( void* pvParameters )
{
	int game_alive;
	tPrint print_data;
	int cant_data;
	BaseType_t result;

	int user_led;

	t_simon* data_user;

	data_user = &simon_data;

	game_alive = 0;

	while( 1 )
	{
		if (!game_alive){
			xSemaphoreTake( sem_sec_ready, portMAX_DELAY );
			game_alive = 1;
			cant_data = 0;
		}

		result =xQueueReceive( simon_data.queue_game, &user_led,  SIMON_USER_INPUT_TIMEOUT );
		if (result == pdTRUE){
			simon_pulse_led_user(user_led);
			xQueueSend( queue_tec_touch, &user_led,  portMAX_DELAY );
			cant_data++;
		}else {
			user_led = SIMON_LED_ID_GAME_OVER;
			xQueueSend( queue_tec_touch, &user_led,  portMAX_DELAY );
		}

		if (cant_data >= simon_data.Nround)
			game_alive=0;
	}
}


void SendUart( tPrint * datos )
{
	switch(datos->status){
		case START_GAME:
			taskENTER_CRITICAL();
			printf( "START GAME \r\n");
			taskEXIT_CRITICAL();
			break;
		case NEW_ROUND:
			taskENTER_CRITICAL();
			printf( "Start a new round \r\n");
			taskEXIT_CRITICAL();
			break;
		case NEXT_ROUND:
			taskENTER_CRITICAL();
			printf( "Go to round %d\r\n",datos->round);
			taskEXIT_CRITICAL();
			break;
		case NEW_TEC:
			taskENTER_CRITICAL();
			printf( "You push the tec %d \r\n",datos->tec_touch+1);
			taskEXIT_CRITICAL();
			break;
		case FINISH_ROUND:
			taskENTER_CRITICAL();
			printf( "Finish round %d\r\n",datos->round);
			taskEXIT_CRITICAL();
			break;
		case GAME_OVER:
			taskENTER_CRITICAL();
			printf( "GAME OVER \r\n");
			taskEXIT_CRITICAL();
			break;
		case WIN:
			taskENTER_CRITICAL();
			printf( "You win the %d rounds of the game in %d mili seconds\r\n",datos->round,datos->time_play*portTICK_RATE_MS);
			taskEXIT_CRITICAL();
			break;
		case GAME_ERROR:
			taskENTER_CRITICAL();
			printf( "Error \r\n");
			taskEXIT_CRITICAL();
			break;
		default:
			taskENTER_CRITICAL();
			printf( "Error, incorrect status\r\n");
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
	//t_simon* data_handler;

	//data_handler = &simon_data;
	int data_to_send;

	// agrego a la cola de usuario los valores que recibe la interrupcion
    switch (event_data->tecla){
		case TEC1_INDEX:
			data_to_send = TEC1_INDEX;
			xQueueSend( simon_data.queue_game, &data_to_send,portMAX_DELAY );
			break;
		case TEC2_INDEX:
			data_to_send = TEC2_INDEX;
			xQueueSend( simon_data.queue_game, &data_to_send,portMAX_DELAY );
			break;
		case TEC3_INDEX:
			data_to_send = TEC3_INDEX;
			xQueueSend( simon_data.queue_game, &data_to_send,portMAX_DELAY );
			break;
		case TEC4_INDEX:
			data_to_send = TEC4_INDEX;
			xQueueSend( simon_data.queue_game, &data_to_send,portMAX_DELAY );
			break;
		default:
			data_to_send = SIMON_LED_ID_GAME_OVER;
			xQueueSend( simon_data.queue_game, &data_to_send,portMAX_DELAY );
			break;
	}
}
