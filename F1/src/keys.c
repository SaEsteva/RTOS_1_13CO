/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/


/*==================[ Inclusions ]============================================*/
#include "keys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"


/*=====[ Definitions of private data types ]===================================*/

/*=====[Definition macros of private constants]==============================*/

#define DEBOUNCE_TIME   40
#define DEBOUNCE_TIME_TICKS     pdMS_TO_TICKS(DEBOUNCE_TIME)

/*=====[Prototypes (declarations) of private functions]======================*/
static void keys_isr_config( void );
static void keys_reset( uint32_t index );
static void keys_event_handler_button_pressed( uint32_t index );
static void keys_event_handler_button_release( uint32_t index );

/*=====[Definitions of private global variables]=============================*/

const t_key_config  keys_config[] = { [TEC1_INDEX]= {TEC1} } ;

#define KEY_COUNT   sizeof(keys_config)/sizeof(keys_config[TEC1_INDEX])

t_key_data keys_data[KEY_COUNT];

/*=====[Definitions of public global variables]==============================*/

/*=====[prototype of private functions]=================================*/
void keys_service_task( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/

/**
   @brief Obtiene el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
   @return TickType_t
 */
TickType_t keys_get_diff( uint32_t index )
{
    TickType_t tiempo;

    taskENTER_CRITICAL();
    tiempo = keys_data[index].time_diff;
    taskEXIT_CRITICAL();

    return tiempo;
}

/**
   @brief Borra el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
 */
void keys_clear_diff( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff = KEYS_INVALID_TIME;
    taskEXIT_CRITICAL();
}

/* funcion no bloqueante que consulta si la tecla fue pulsada. */
int key_pressed( uint32_t index )
{
    BaseType_t signaled = xSemaphoreTake( keys_data[index].pressed_signal, 0 );
    if ( signaled == pdPASS )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
   @brief Inicializa el driver

 */
void keys_init( void )
{
    BaseType_t res;

    for ( uint32_t i = 0 ; i < KEY_COUNT ; i++ )
    {
        keys_data[i].state          = BUTTON_UP;  // Set initial state
        keys_data[i].time_down      = KEYS_INVALID_TIME;
        keys_data[i].time_up        = KEYS_INVALID_TIME;
        keys_data[i].time_diff      = KEYS_INVALID_TIME;

        keys_data[i].isr_signal     = xSemaphoreCreateBinary();
        keys_data[i].pressed_signal = xSemaphoreCreateBinary();

        configASSERT( keys_data[i].isr_signal != NULL );
        configASSERT( keys_data[i].pressed_signal != NULL );
    }

    // Crear tareas en freeRTOS
    res = xTaskCreate (
              keys_service_task,					// Funcion de la tarea a ejecutar
              ( const char * )"keys_service_task",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
              0,							// Parametros de tarea
              tskIDLE_PRIORITY+1,			// Prioridad de la tarea
              0							// Puntero a la tarea creada en el sistema
          );

#if KEYS_USE_ISR==1
    keys_isr_config();
#endif

    // Gestión de errores
    configASSERT( res == pdPASS );
}

#if KEYS_USE_ISR==0

/**
   @brief Keys SM for polled operation

   @param index
 */
void keys_Update( uint32_t index )
{
    switch( keys_data[index].state )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_DOWN;

                /* ACCION DEL EVENTO !*/
                keys_event_handler_button_pressed( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_UP;
            }

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
            /* CHECK TRANSITION CONDITIONS */
            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_RISING;
            }
            break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_UP;

                /* ACCION DEL EVENTO ! */
                keys_event_handler_button_release( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_DOWN;
            }

            /* LEAVE */
            break;

        default:
            keys_reset( index );
            break;
    }
}
#else

/**
   @brief Keys SM for isr operation

   @param index
 */
void keys_Update_Isr( uint32_t index )
{
    switch( keys_data[index].state )
    {
        case STATE_BUTTON_UP:

            /* espero una señal de la tecla */
            xSemaphoreTake( keys_data[index].isr_signal, portMAX_DELAY );

            /* la tecla se pulso */
            keys_data[index].state = STATE_BUTTON_FALLING;
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_DOWN;

                /* ACCION DEL EVENTO !*/
                keys_event_handler_button_pressed( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_UP;
            }

            //desenmascarar Chip_PININT_EnableIntLow

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:

            /* espero una señal de la tecla */
            xSemaphoreTake( keys_data[index].isr_signal, portMAX_DELAY );

            /* la tecla se pulsó */
            keys_data[index].state = STATE_BUTTON_RISING;

            break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_UP;

                /* ACCION DEL EVENTO ! */
                keys_event_handler_button_release( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_DOWN;
            }

            /* LEAVE */
            break;

        default:
            keys_reset( index );
            break;
    }
}
#endif


/*=====[Implementations of private functions]================================*/

/**
   @brief   Manejador del evento de tecla pulsada

   @param event_data
 */
static void keys_event_handler_button_pressed( uint32_t index )
{
    TickType_t current_tick_count = xTaskGetTickCount();

    taskENTER_CRITICAL();
    keys_data[index].time_down = current_tick_count;
    taskEXIT_CRITICAL();
}

/**
   @brief   Manejador del evento de tecla liberada

   @param event_data
 */
static void keys_event_handler_button_release( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
    taskEXIT_CRITICAL();

    xSemaphoreGive( keys_data[index].pressed_signal ) ;
}

/**
   @brief Restarts the button SM

   @param index
 */
static void keys_reset( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].state = BUTTON_UP;
    taskEXIT_CRITICAL();

    /* TODO restore semaphore states for isr */
}

/*=====[Implementations of private functions]=================================*/

/**
   @brief Implementacion de tarea que le da servicio al driver

   @param taskParmPtr
 */
void keys_service_task( void* taskParmPtr )
{
    while( TRUE )
    {
#if KEYS_USE_ISR==0
        keys_Update( TEC1_INDEX );
        vTaskDelay( DEBOUNCE_TIME / portTICK_RATE_MS );
#else
        keys_Update_Isr( TEC1_INDEX );
        vTaskDelay( DEBOUNCE_TIME / portTICK_RATE_MS );
#endif
    }
}



#if KEYS_USE_ISR==1
/**
   @brief   Inicializa las interrupciones asociadas al driver keys.c
			Se realiza sobre las cuatro teclas de la EDUCIAA
 */
void keys_isr_config( void )
{
    //Inicializamos las interrupciones (LPCopen)
    Chip_PININT_Init( LPC_GPIO_PIN_INT );

    //Inicializamos de cada evento de interrupcion (LPCopen)

    /* Machete:
    GLOBAL! extern pinInitGpioLpc4337_t gpioPinsInit[];
    Chip_SCU_GPIOIntPinSel( j,  gpioPinsInit[i].gpio.port, gpioPinsInit[i].gpio.pin );   // TECi
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj (canal j -> hanlder GPIOj)       //Borra el pending de la IRQ
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj //Selecciona activo por flanco
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( j ) );                        // INTj //Selecciona activo por flanco descendente
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( j ) );                       // INTj //Selecciona activo por flanco ascendente
    */

    /*Seteo la interrupción para el flanco descendente
                        channel, GPIOx,                        [y]    <- no es la config del pin, sino el nombre interno de la señal
                           |       |                           |
                           |       |                           |    */

    // TEC1 FALL
    Chip_SCU_GPIOIntPinSel( 0, 0, 4 ); 	//(Canal 0 a 7, Puerto GPIO, Pin GPIO)
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura el canal para que se active por flanco
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura para que el flanco sea el de bajada

    // TEC1 RISE
    Chip_SCU_GPIOIntPinSel( 1, 0, 4 );	//(Canal 0 a 7, Puerto GPIO, Pin GPIO)
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH1 ); //Se configura el canal para que se active por flanco
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH1 ); //En este caso el flanco es de subida

#if 0 //para mas de 1 tecla, extender
    // TEC2 FALL
    Chip_SCU_GPIOIntPinSel( 2, 0, 8 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH2 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH2 );

    // TEC1 RISE
    Chip_SCU_GPIOIntPinSel( 3, 0, 8 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH3 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH3 );

    // TEC3 FALL
    Chip_SCU_GPIOIntPinSel( 4, 0, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH4 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH4 );

    // TEC1 RISE
    Chip_SCU_GPIOIntPinSel( 5, 0, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH5 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH5 );

    // TEC4 FALL
    Chip_SCU_GPIOIntPinSel( 6, 1, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH6 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH6 );

    // TEC4 RISE
    Chip_SCU_GPIOIntPinSel( 7, 1, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH7 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH7 );

#endif

    //Una vez que se han configurado los eventos para cada canal de interrupcion
    //Se activan las interrupciones para que comiencen a llamar al handler
    NVIC_SetPriority( PIN_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_SetPriority( PIN_INT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT1_IRQn );

#if 0 //para mas de 1 tecla, extender
    NVIC_SetPriority( PIN_INT2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_SetPriority( PIN_INT3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
    NVIC_SetPriority( PIN_INT4_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT4_IRQn );
    NVIC_SetPriority( PIN_INT5_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT5_IRQn );
    NVIC_SetPriority( PIN_INT6_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT6_IRQn );
    NVIC_SetPriority( PIN_INT7_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT7_IRQn );
#endif
}

/**
   @brief handler de evento de tecla pulsada

   @param index
 */
void keys_isr_fall( uint32_t index )
{
    UBaseType_t uxSavedInterruptStatus;

    /* esta operacion debe realizarse en zona critica. Recordar que el objeto global puede estar leyendose
       o escribiendose en otro contexto  */
    uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    keys_data[index].time_down = xTaskGetTickCountFromISR();
    taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
}

/**
   @brief handler de evento de tecla liberada

   @param index
 */
void keys_isr_rise( uint32_t index )
{
    UBaseType_t uxSavedInterruptStatus;

    /* esta operacion debe realizarse en zona critica. Recordar que el objeto global puede estar leyendose
       o escribiendose en otro contexto  */
    uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    keys_data[index].time_up = xTaskGetTickCountFromISR();
    taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
}

void GPIO0_IRQHandler( void )   //asociado a tec1
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 ) //Verificamos que la interrupciÃ³n es la esperada
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 ); //Borramos el flag de interrupciÃ³n

        keys_isr_fall( TEC1_INDEX );

        xSemaphoreGiveFromISR( keys_data[TEC1_INDEX].isr_signal, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO1_IRQHandler( void )  //asociado a tec1
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH1 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH1 );

        keys_isr_rise( TEC1_INDEX );

        xSemaphoreGiveFromISR( keys_data[TEC1_INDEX].isr_signal, &xHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


#if 0 //extender a mas isr
void GPIO2_IRQHandler( void )
{
    UBaseType_t uxSavedInterruptStatus;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH2 ) //Verificamos que la interrupciÃ³n es la esperada
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH2 ); //Borramos el flag de interrupciÃ³n

        /* proceso */
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO3_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH3 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH3 );
        //codigo a ejecutar si ocurriÃ³ la interrupciÃ³n

        /* proceso */
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO4_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable


    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH4 ) //Verificamos que la interrupciÃ³n es la esperada
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH4 ); //Borramos el flag de interrupciÃ³n

        /* proceso */
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO5_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH5 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH5 );

        /* proceso */
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO6_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH6 ) //Verificamos que la interrupciÃ³n es la esperada
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH6 ); //Borramos el flag de interrupciÃ³n

        /* proceso */
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO7_IRQHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH7 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH7 );

        /* proceso */
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

#endif

#endif
