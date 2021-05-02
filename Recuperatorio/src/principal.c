
/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"
#include "keys.h"
#include "time_tec.h"
#include "principal.h"

/*=====[Definition & macros of public constants]==============================*/

/*=====[Definitions of extern global functions]==============================*/


t_led_struct sec_led_struct;

t_tecla_led leds_data[] = { {.led= LEDB}};

gpioMap_t gpio_t[] = {GPIO7, GPIO5, GPIO3, GPIO1};

TickType_t time_control;

int Modo1 [] = {0, 1,2,3,4,5,6,7,8,9,10};
int Modo2 [] = {10,9,8,7,6,5,4,3,2,1,0};
int Modo3 [] = {0,1,2,3,4,5,6,7,8,9,10,9,8,7,6,5,4,3,2,1};
int ModoOff [] = {0,0,0,0,0,0,0,0,0,0,0};
int ModoError [] = {5,5,5,5,5,5,5,5,5,5,5};

#define LED_COUNT   sizeof(leds_data)/sizeof(leds_data[0])

/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/


void init_struct(){
	time_control = T_MIN_TICKS;
	sec_led_struct.actual_status = MODO1_SEC;
}

void principal_init( void )
{
    BaseType_t res;

    init_struct();

    gpioInit( gpio_t[0], GPIO_OUTPUT );

    res = xTaskCreate (
			  task_led,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_led",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0							// Puntero a la tarea creada en el sistema
		  );

	// Gestión de errores
	configASSERT( res == pdPASS );

    res = xTaskCreate (
    		task_button,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_button",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  &leds_data[0],							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0							// Puntero a la tarea creada en el sistema
		  );

	// Gestión de errores
	configASSERT( res == pdPASS );

}

void user_keys_event_handler_button_pressed( t_key_isr_signal* event_data )
{
    //nada
}

void user_keys_event_handler_button_release( t_key_isr_signal* event_data )
{
	// agrego a la cola de usuario los valores que recibe la interrupcion
    switch (event_data->tecla){
		case TEC1_INDEX:
			 xQueueSend( leds_data[0].queue, event_data, portMAX_DELAY );
			break;
		case TEC2_INDEX:
			inc_counter();
			break;
		case TEC3_INDEX:
			dec_counter();
			break;
		default:
			init_struct();
			break;
	}
}


void task_led( void* taskParmPtr )
{
    TickType_t t_on =   pdMS_TO_TICKS( 10 );

    TickType_t xPeriodicity = pdMS_TO_TICKS( 1000 ); // Tarea periodica cada 1000 ms

    TickType_t xLastWakeTime = xTaskGetTickCount();

    int index = 0;
    int modo_selec;
    int max_num = 10;

    while( 1 )
    {
    	if (sec_led_struct.actual_status == MODO3_SEC){
			if (index >= N_MODE2)
				index = 0;
		}else{
			if (index >= N_MODE)
				index = 0;
		}

    	switch(sec_led_struct.actual_status){
    			case MODO1_SEC:
    				modo_selec = Modo1[index];
    				index++;
    				break;
    			case MODO2_SEC:
    				modo_selec = Modo2[index];
					index++;
    				break;
    			case MODO3_SEC:
    				modo_selec = Modo3[index];
					index++;
    				break;
    			case MODOOFF_SEC:
					modo_selec = ModoOff[index];
					index++;
					break;
				default:
    				modo_selec = ModoError[index];
					index++;
    				break;
		}

    	taskENTER_CRITICAL();
    	t_on = (TickType_t)((time_control*modo_selec)/max_num);
    	xPeriodicity = time_control;
		taskEXIT_CRITICAL();

    	if (t_on > 0){
			gpioWrite( leds_data[0].led, ON );
			gpioWrite( gpio_t[0], ON );
			vTaskDelay( t_on );
		}
		gpioWrite( leds_data[0].led, OFF );
		gpioWrite( gpio_t[0], OFF );
        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
    }
}


void task_button( void* taskParmPtr )
{
    t_tecla_led* led_data = taskParmPtr;

    t_key_isr_signal evnt;

    TickType_t dif =   pdMS_TO_TICKS( 500 );

    /* creo cola para que la tarea reciba las pulsaciones de la tecla 1 */
    leds_data[0].queue = xQueueCreate( 2, sizeof( t_key_isr_signal ) );

    if( led_data->queue == NULL )
	{
		gpioWrite( LED_ERROR, ON );
		printf( MSG_ERROR_QUE );
		while( TRUE );						// VER ESTE LINK: https://pbs.twimg.com/media/BafQje7CcAAN5en.jpg
	}
    int index_nreset = 0;
    while( 1 )
    {

    	xQueueReceive( led_data->queue, &evnt, portMAX_DELAY );

        dif = keys_get_diff( evnt.tecla );


        if (dif >= T1_MAX_TICKS){
        	change_status(MODOREST_SEC,index_nreset);
        	index_nreset++;
        }else{
        	change_status(sec_led_struct.actual_status,0);
        	index_nreset = 0;
        }
    }
}

void change_status(t_modo_status status,int n_reset){
	switch(status){
			case MODO1_SEC:
				taskENTER_CRITICAL();
				sec_led_struct.last_status = MODO1_SEC;
				sec_led_struct.actual_status = MODO2_SEC;
				taskEXIT_CRITICAL();
				break;
			case MODO2_SEC:
				taskENTER_CRITICAL();
				sec_led_struct.last_status = MODO2_SEC;
				sec_led_struct.actual_status = MODO3_SEC;
				taskEXIT_CRITICAL();
				break;
			case MODO3_SEC:
				taskENTER_CRITICAL();
				sec_led_struct.last_status = MODO3_SEC;
				sec_led_struct.actual_status = MODO1_SEC;
				taskEXIT_CRITICAL();
				break;
			case MODOOFF_SEC:
				taskENTER_CRITICAL();
				sec_led_struct.actual_status = sec_led_struct.last_status;
				taskEXIT_CRITICAL();
				break;
			case MODOREST_SEC:
				if (n_reset == 0){
					taskENTER_CRITICAL();
					sec_led_struct.last_status = sec_led_struct.actual_status;
					taskEXIT_CRITICAL();
				}
				taskENTER_CRITICAL();
				sec_led_struct.actual_status = MODOOFF_SEC;
				taskEXIT_CRITICAL();
				break;
			default:
				taskENTER_CRITICAL();
				sec_led_struct.last_status = sec_led_struct.actual_status;
				sec_led_struct.actual_status = MODOERROR_SEC;
				taskEXIT_CRITICAL();
				break;
	}
}
