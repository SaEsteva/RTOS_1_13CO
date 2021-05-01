/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/23/04
 * Version: v1.2
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "../inc/tarea.h"

#include "FreeRTOS.h"
#include "sapi.h"


/*=====[ Definitions of private data types ]===================================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Definitions of private global variables]=============================*/


/*=====[Definitions of public global variables]==============================*/
extern t_key_config* keys_config;
extern t_key_data* keys_data;
/*=====[prototype of private functions]=================================*/
void print(imprimir_t* print_struct){
	TickType_t diff;

	if (print_struct->time_print){
		taskENTER_CRITICAL();
		diff = keys_data[print_struct->second_val]->time_up - keys_data[print_struct->first_val]->time_up;
		taskEXIT_CRITICAL();
	}else{
		taskENTER_CRITICAL();
		diff = keys_data[print_struct->second_val]->time_down - keys_data[print_struct->first_val]->time_down;
		taskEXIT_CRITICAL();
	}


	taskENTER_CRITICAL();
	printf( "{%d:%d:%d}\r\n",print_struct->first_val,print_struct->second_val,diff*portTICK_RATE_MS );
	taskEXIT_CRITICAL();

}

void tarea( void* taskParmPtr ){
	tarea_MEF_t mef_state;
	imprimir_t print_struct;

	print_struct.printer=0;
	mef_state = INIT;
	while(1){


		switch (mef_state){
			case INIT:
				if( key_pressed( 0 ) )
				{
					mef_state = BUTTON_0_DOWN;
				}else if( key_pressed( 1 ) )
				{
					mef_state = BUTTON_1_DOWN;
				}
				break;
			case BUTTON_0_DOWN:
				if( key_pressed( 1 ) )
				{
					mef_state = BUTTONS_DOWN;
					print_struct.first_val=0;
					print_struct.second_val=1;
					print_struct.printer = 1;
				}else if ( key_released( 0 ) )
				{
					mef_state = INIT;
				}
				break;
			case BUTTON_1_DOWN:
				if( key_pressed( 0 ) )
				{
					mef_state = BUTTONS_DOWN;
					print_struct.first_val=1;
					print_struct.second_val=0;
					print_struct.printer = 1;
				}else if ( key_released( 1 ) )
				{
					mef_state = INIT;
				}
				break;
			case BUTTONS_DOWN:
				if (print_struct.printer){
					print_struct.time_print=0;
					print(&print_struct);
					print_struct.printer = 0;
				}
				if( key_released( 0 ) )
				{
					mef_state = BUTTON_0_UP;
				}else if( key_released( 1 ) )
				{
					mef_state = BUTTON_0_UP;
				}
				break;
			case BUTTON_0_UP:
				if( key_released( 1 ) )
				{
					mef_state = BUTTONS_UP;
					print_struct.first_val=0;
					print_struct.second_val=1;
				}else if ( key_pressed( 0 ) )
				{
					mef_state = INIT;
				}
				break;
			case BUTTON_1_UP:
				if( key_released( 0 ) )
				{
					mef_state = BUTTONS_UP;
					print_struct.first_val=0;
					print_struct.second_val=1;
				}else if ( key_pressed( 1 ) )
				{
					mef_state = INIT;
				}
				break;
			case BUTTONS_UP:
				print_struct.time_print=1;
				print(&print_struct);
				mef_state = INIT;
				break;
			default:
				mef_state = INIT;
				break;
		}
	}
}

