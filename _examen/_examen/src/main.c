/*=============================================================================
 * Copyright (c) 2021, Joaquin Gaspar Ulloa <joaquin.g.ulloa@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/04/23
 * Version: v1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "../inc/pwm.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"

#include "print.h"
#include "keys.h"
#include "random.h"


/*=====[Definition & macros of public constants]==============================*/

/*=====[Definitions of extern global functions]==============================*/


/*=====[Definitions of public global variables]==============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	// Se inicializa la plataforma
    boardConfig();

    printf( MSG_BOOT );

    // Se inicializa driver de leds
    // leds_init();

    // Se inicializa driver de teclas
    keys_init();

    // Se inicializa el juego
    pwm_init();

    // Se inicializa scheduler
    vTaskStartScheduler();

    // Equivalente al while(TRUE)
    configASSERT( 0 );

    // Nunca llega aqui
    return 0;
}


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( MSG_MALLOC_ERROR );
    configASSERT( 0 );
}
