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
#include "protocol.h"
#include "semphr.h"

#define FRAME_MAX_SIZE  200

uartMap_t         uart_used;
SemaphoreHandle_t new_frame_signal;
SemaphoreHandle_t mutex;

char buffer[FRAME_MAX_SIZE];
uint16_t index;


/**
   @brief Manejador del evento de recepcion de 1 byte via UART
   @param noUsado
 */
void protocol_rx_event( void *noUsado )
{
    ( void* ) noUsado;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* leemos el caracter recibido */
    char c = uartRxRead( UART_USB );

    BaseType_t signaled = xSemaphoreTakeFromISR( mutex, &xHigherPriorityTaskWoken );

    if( signaled )
    {
        if( FRAME_MAX_SIZE-1==index )
        {
            /* reinicio el paquete */
            index = 0;
        }

        if( c=='>' )
        {
            if( index==0 )
            {
                /* 1er byte del frame*/
            }
            else
            {
                /* fuerzo el arranque del frame (descarto lo anterior)*/
                index = 0;
            }

            buffer[index] = c;

            /* incremento el indice */
            index++;
        }
        else if( c=='<' )
        {
            /* solo cierro el fin de frame si al menos se recibio un start.*/
            if( index>=1 )
            {
                /* se termino el paquete - guardo el dato */
                buffer[index] = c;

                /* incremento el indice */
                index++;

                /* Deshabilito todas las interrupciones de UART_USB */
                uartCallbackClr( uart_used, UART_RECEIVE );

                /* señalizo a la aplicacion */
                xSemaphoreGiveFromISR( new_frame_signal, &xHigherPriorityTaskWoken );
            }
            else
            {
                /* no hago nada, descarto el byte */
            }
        }
        else
        {
            /* solo cierro el fin de frame si al menos se recibio un start.*/
            if( index>=1 )
            {
                /* guardo el dato */
                buffer[index] = c;

                /* incremento el indice */
                index++;
            }
            else
            {
                /* no hago nada, descarto el byte */
            }
        }

        xSemaphoreGiveFromISR( mutex, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
   @brief Inicializa el stack

   @param uart
   @param baudRate
 */
void procotol_x_init( uartMap_t uart, uint32_t baudRate )
{
    /* CONFIGURO EL DRIVER */

    uart_used = uart;

    /* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
    uartConfig( uart, baudRate );

    /* Seteo un callback al evento de recepcion y habilito su interrupcion */
    uartCallbackSet( uart, UART_RECEIVE, protocol_rx_event, NULL );

    /* Habilito todas las interrupciones de UART_USB */
    uartInterrupt( uart, true );

    /* CONFIGURO LA PARTE LOGICA */
    index = 0;
    new_frame_signal = xSemaphoreCreateBinary();
    mutex = xSemaphoreCreateMutex();
}

/**
   @brief Espera indefinidamente por un paquete.
 */
void protocol_wait_frame()
{
    xSemaphoreTake( new_frame_signal , portMAX_DELAY );
    xSemaphoreTake( mutex, 0 );
}

/**
   @brief Devuelve la referencia al paquete recibido

   @param data
   @param size
 */
void  protocol_get_frame_ref( char** data, uint16_t* size )
{
    *data = buffer;
    *size = index;
}


/**
   @brief Una vez procesado el frame, descarta el paquete y permite una nueva recepcion.
 */
void protocol_discard_frame()
{
    /* indico que se puede inciar un paquete nuevo */
    index = 0;

    /* libero la seccion critica, para que el handler permita ejecutarse */
    xSemaphoreGive( mutex );

    /* limpio cualquier interrupcion que hay ocurrido durante el procesamiento */
    uartClearPendingInterrupt( uart_used );

    /* habilito isr rx  de UART_USB */
    uartCallbackSet( uart_used, UART_RECEIVE, protocol_rx_event, NULL );
}