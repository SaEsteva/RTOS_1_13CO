# Resolución de ejercicio A2 de Guia de ejercitación de RTOS de CESE (2020)

escribir un programa en donde 3 tareas controlen 3 leds

Led 1 - Tarea A - Prioridad +3
Led 2 - Tarea B - Prioridad +2
Led 3 - Tarea C - Prioridad +1
Led 4 - Tarea D - Prioridad +0

Generar una secuencia de LEDS segun este patron y luego, finalizar el RTOS

2  2  2  1  1  3  1  4  4  4  4  4  4 ....

El sistema deberá arrancar ejecutando la tarea de mayor prioridad solamente.

Solo la tarea 4 puede matar al resto, cuando comience a operar. 

¿que sucede si apago round robin?

 



It works on these platform/envioroment:

* Linux
* Windows + CYGWIN
* Windows + MSYS2


Configure program.mk file on the root of this repository the following line:

`CONFIG_APP_DIR?=examples/c/freertos/A2/educiaa`

or

`CONFIG_APP_DIR?=examples/c/freertos/A2/x86`