/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"

#include "keys.h"
#include "random.h"
#include "simon.h"

int main( void )
{
    /* Inicializar la placa */
    boardConfig();

    printf( "\nBOOTING GAME\n" );

    /* inicializo driver de teclas */

    simon_init();
    keys_init();

    vTaskStartScheduler();

    configASSERT( 0 );
    return 0;
}
