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

#include "semphr.h"



void wait_frame( void* pvParameters )
{
    char* data;
    uint16_t size;

    uint16_t frame_counter = 0;

    while( TRUE )
    {
        protocol_wait_frame();

        protocol_get_frame_ref( &data, &size );

        /* para el printf */
        data[size] = '\0';
        printf( "%s %u\n", data, frame_counter );

        protocol_discard_frame();

        /* hago un blink para que se vea */
        gpioToggle( LEDB );
        vTaskDelay( 100/portTICK_RATE_MS );
        gpioToggle( LEDB );

        frame_counter++;
    }
}


int main( void )
{
    /* Inicializar la placa */
    boardConfig();

    procotol_x_init( UART_USB, 115200 );


    xTaskCreate(
        wait_frame,                  // Funcion de la tarea a ejecutar
        ( const char * )"wait_frame", // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2,   // Cantidad de stack de la tarea
        0,                            // Parametros de tarea
        tskIDLE_PRIORITY+1,           // Prioridad de la tarea
        0                             // Puntero a la tarea creada en el sistema
    );

    vTaskStartScheduler();

    return 0;
}
