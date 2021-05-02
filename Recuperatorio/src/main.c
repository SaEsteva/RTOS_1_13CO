
/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"
#include "keys.h"
#include "time_tec.h"
#include "principal.h"

/*=====[Definition & macros of public constants]==============================*/

/*=====[Definitions of extern global functions]==============================*/

// Prototipo de funcion de la tarea

int main( void )
{

    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();  // Inicializar y configurar la plataforma

    printf( "Recuperatorio Esteva Santiago\r\n" );

    /* inicializo driver de teclas */
    keys_init();
    principal_init();

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    /* realizar un assert con "false" es equivalente al while(1) */
    configASSERT( 0 );
    return 0;
}


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}
