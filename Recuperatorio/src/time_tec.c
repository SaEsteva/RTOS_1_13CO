
/*==================[ Inclusions ]============================================*/
#include "time_tec.h"

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/*=====[ Definitions of private data types ]===================================*/

/*=====[Definition macros of private constants]==============================*/



/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Definitions of public global variables]==============================*/
extern TickType_t time_control;

/*=====[Implementations of public functions]=================================*/


void dec_counter(){
   TickType_t aux_tick_count;
   
   taskENTER_CRITICAL();
   aux_tick_count = time_control;
   taskEXIT_CRITICAL();
   
   if (aux_tick_count <= T_MIN_TICKS)
      aux_tick_count = T_MIN_TICKS;
   else
	   aux_tick_count = aux_tick_count - T_INCREMENTAL_TICKS;
      
   taskENTER_CRITICAL();
   time_control = aux_tick_count;
   taskEXIT_CRITICAL();
}

void inc_counter(){
   TickType_t aux_tick_count ;
   
   taskENTER_CRITICAL();
   aux_tick_count = time_control;
   taskEXIT_CRITICAL();
   
   if (aux_tick_count >= T_MAX_TICKS)
      aux_tick_count = T_MAX_TICKS;
   else
	   aux_tick_count = aux_tick_count + T_INCREMENTAL_TICKS;

   taskENTER_CRITICAL();
   time_control = aux_tick_count;
   taskEXIT_CRITICAL();
}
