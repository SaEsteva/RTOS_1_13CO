/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/18/03
 * Version: v1.0
 *===========================================================================*/
/*==================[ Inclusions ]============================================*/
#include "UART_SO.h"

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/*=====[ Definitions of private data types ]===================================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Prototypes (declarations) of private functions]======================*/



/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/
uint8_t byte_a_transmitir = 203;

/*=====[prototype of private functions]=================================*/
uint32_t DATA_RATE;
uint32_t BIT_RATE;

/*=====[Implementations of public functions]=================================*/

void sw_uart_config( uint16_t taskParmPtr ){
   
   uint32_t baurate = (uint32_t) taskParmPtr;
   BaseType_t res;
   
   //creo la tarea de enviar dato
   res = xTaskCreate (
	  sw_uart_sent,					// Funcion de la tarea a ejecutar
	  ( const char * )"sw_uart_sent",	// Nombre de la tarea como String amigable para el usuario
	  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
	  byte_a_transmitir,							// Parametros de tarea
	  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
	  0							// Puntero a la tarea creada en el sistema
	);

	// Gestión de errores
	configASSERT( res == pdPASS );
   
   //actualiza el baudrate de la uart
   DATA_RATE = baurate;
   BIT_RATE = DATA_RATE/BITS_DATA;
   
   vTaskDelete(NULL);
}

void sw_uart_sent( uint8_t taskParmPtr ){
	uint8_t data = taskParmPtr;
    // ---------- CONFIGURACIONES ------------------------------
	
    // ---------- REPETIR POR # BITS --------------------------
   uint32_t i = 0;
   bool_t bit ;
   
   while( i < BITS_DATA+1 )
    {
    	if (i == 0){ //start
         gpioWrite( GPIO7 , OFF );
      }
      else if (i == BITS_DATA+1){ //stop
         gpioWrite( GPIO7 , ON );
      }
      else{ //send data
         bit = (data & 1 << i)>>i;
         if (bit == 1 ){ //send 1
            gpioWrite( GPIO7 , ON );
         } 
         else { //send 0
            gpioWrite( GPIO7 , OFF );
         }         
      }               
      vTaskDelay( BIT_RATE );       
    }
    vTaskDelete(NULL);
}


uint8_t sw_uart_receive( uint32_t taskParmPtr ){
	
   uint8_t timeout = taskParmPtr;
    // ---------- CONFIGURACIONES ------------------------------
	
    // ---------- REPETIR POR # BITS --------------------------
   uint32_t i = 0;
   bool_t bit ;
   
   while( i < BITS_DATA+1 )
    {
    	if (i == 0){ //start
         gpioWrite( GPIO7 , OFF );
      }
      else if (i == BITS_DATA+1){ //stop
         gpioWrite( GPIO7 , ON );
      }
      else{ //send data
         bit = (data & 1 << i)>>i;
         if (bit == 1 ){ //send 1
            gpioWrite( GPIO7 , ON );
         } 
         else { //send 0
            gpioWrite( GPIO7 , OFF );
         }         
      }               
      vTaskDelay( BIT_RATE );       
    }
    vTaskDelete(NULL);
}