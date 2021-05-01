#include "simon.h"



QueueHandle_t queue_sec_ready,queue_data_ready;


// Estructura con todos los datos
t_simon simon_data;


t_led leds_data[] = { {.led= LEDB}, {.led= LED1}, {.led= LED2}, {.led= LED3}};
#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])


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
	queue_data_ready = xQueueCreate( 1, sizeof( t_led ) );

	// Gestion de errores de colas
	if( queue_sec_ready == NULL || queue_data_ready == NULL )
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

void init_game_data(){
	t_simon* data_to_init;

	data_to_init = &simon_data;

	data_to_init->Nround = 0;
	data_to_init->data_to_send = 0;
	data_to_init->new_round = 1;
	data_to_init->status = NEW_ROUND;
	data_to_init->time_game = xTaskGetTickCount();
	data_to_init->queue_user = xQueueCreate( SIMON_MAX_ROUNDS, sizeof( t_led ) ); //creo la cola que recibe esto
	data_to_init->queue_game = xQueueCreate( SIMON_MAX_ROUNDS, sizeof( t_led ) ); //creo la cola que recibe esto

	if( data_to_init->queue_user == NULL || data_to_init->queue_game == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_QUE );
		while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}

}

void simon_service_logic( void* pvParameters )
{
	int game_alive;
	int round;
	char leds_rounds[SIMON_MAX_ROUNDS];
	BaseType_t result;

	tPrint print_data;

	game_alive = 0;

	t_led led_sec,led_send;

	t_simon* data_logic;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	init_game_data();

	data_logic = &simon_data;



	while( 1 )
	{
		if (!game_alive){
			xQueueReceive( data_logic->queue_user, &led_sec, portMAX_DELAY );
			xQueueReset(data_logic->queue_user);
			xQueueReset(data_logic->queue_game);
			game_alive = 1;
			data_logic->new_round = 1;
		}

		if (data_logic->new_round){
			data_logic->Nround++;
			for (round = 0 ; round < SIMON_MAX_ROUNDS; round++ ){
				led_sec = leds_data[1];
				xQueueSend( data_logic->queue_game, &led_sec,  portMAX_DELAY );
				simon_pulse_led(1);
			}
			data_logic->time_game = xTaskGetTickCount();
			round = 0;
			data_logic->new_round = 0;
			xQueueSend( queue_sec_ready, &game_alive,  0 );
		}

       	result = xQueueReceive( queue_data_ready, &led_send,  0 );
		if (result == pdTRUE){
			xQueueReceive( data_logic->queue_user, &led_send,  0 );
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
			xQueueSend( queue_sec_ready, &game_alive,  0 );
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
	int cant_data;

	t_led user_led;

	t_simon* data_user;

	data_user = &simon_data;

	game_alive = 0;

	while( 1 )
	{
		xQueueReceive( queue_sec_ready, &game_alive,  portMAX_DELAY );

		xQueueReceive( data_user->queue_user, &user_led,  portMAX_DELAY );


		xQueueSend( queue_data_ready, &user_led,  portMAX_DELAY );
		simon_pulse_led_user(user_led.led);
		print_data.status=NEW_TEC;
		print_data.tec_touch=user_led.led;
		SendUart(&print_data);
		//game_alive = 0;



	}
}


void SendUart( tPrint * datos )
{
          switch(datos->status){
        	case GAME_OVER:
        		taskENTER_CRITICAL();
        		printf( "GAME OVER \r\n");
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
	t_simon* data_handler;

	data_handler = &simon_data;

	t_led* leds_touch;

	// agrego a la cola de usuario los valores que recibe la interrupcion
    if( event_data->tecla == TEC1_INDEX )
    {
    	leds_touch = &leds_data[0];
    	xQueueSend( data_handler->queue_user, leds_touch,portMAX_DELAY );
    }

    if( event_data->tecla == TEC2_INDEX )
    {
    	leds_touch = &leds_data[1];
    	xQueueSend( data_handler->queue_user, leds_touch,portMAX_DELAY );
    }

    if( event_data->tecla == TEC3_INDEX )
    {
    	leds_touch = &leds_data[2];
		xQueueSend( data_handler->queue_user, leds_touch,portMAX_DELAY );
    }

    if( event_data->tecla == TEC4_INDEX )
    {
    	leds_touch = &leds_data[3];
		xQueueSend( data_handler->queue_user, leds_touch,portMAX_DELAY );
    }
}
