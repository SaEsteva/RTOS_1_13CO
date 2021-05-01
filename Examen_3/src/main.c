/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"

#include "keys.h"
#include "random.h"
#include "simon.h"


/*=====[Definition & macros of public constants]==============================*/

/*=====[Definitions of extern global functions]==============================*/


/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
    /* Inicializar la placa */
    boardConfig();

    printf( "\nBOOTING GAME\r\n" );

    /* inicializo driver de teclas */

    keys_init();
    simon_init();
    

    vTaskStartScheduler();

    configASSERT( 0 );
    return 0;
}


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}
