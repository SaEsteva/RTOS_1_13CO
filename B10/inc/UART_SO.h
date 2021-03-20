/*=============================================================================
 * Copyright (c) 2021, Santiago Esteva <sesteva@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/18/03
 * Version: v1.0
 *===========================================================================*/

#ifndef UART_SO_H_
#define UART_SO_H_

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/* public macros ================================================================= */
#define BITS_DATA  8
/* types ================================================================= */

/* methods ================================================================= */
void sw_uart_config( uint16_t );
void sw_uart_sent( uint8_t );
#endif /* RTOSI_UART_GPIO_H_ */