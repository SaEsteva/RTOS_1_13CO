#include "stdlib.h"


#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#endif

/**
   @brief establece la semilla del pseudorandom generator

   @param semilla
 */
void random_set_seed( uint32_t semilla )
{
    if ( semilla != 0 )
    {
        srand( semilla ) ;
    }
}

/**
   @brief   Genera un numero aleatorio entre min y max, incluidos
            Pueden utilizarse numeros negativos

   @param min
   @param max
   @return uint32_t
 */
uint32_t random( int32_t min, int32_t max )
{
    if ( min>=max )
    {
        return min ;
    }

    //printf( "%d %d %d\n", min, max, ( max-min ) + min );

    return rand() % ( max-min ) + min;
}

#ifdef USE_FREERTOS
/**
   @brief genera una semilla nueva usando el tick del sistema al momento en que se llama.

 */
void random_seed_freertos()
{
    random_set_seed( xTaskGetTickCount() );
}
#endif