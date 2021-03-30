# Resolucion del ejercicio D1

La resolucion presentada difiere de la presentada en clase en los siguientes aspectos:

- Se modularizó la funcionalidad relacionada al "driver de teclas" (quedaron algunos aspectos para mejorar)
- La máquina de estados de las teclas se redujo.
- Se dejó el uso de printf para enviar mensajes de depuración al puerto UART_USB (se configura a 115200 antes de la ejecución del main)
- Se separa el modelo de datos de cada tecla en datos configurable (constante) y datos en RAM.


# Preguntas para el alumno:

- ¿Porque no se protege con zona critica el acceso al campo "state" de cada una de las teclas ?
- ¿Es realmente necesario proteger TODOS los accesos a los campos de keys_data[ ] ?

