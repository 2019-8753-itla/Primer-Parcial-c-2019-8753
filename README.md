Primer Parcial de Microcontroladores

* Nombre: Jose Amauris Palma Rosario
* Matrícula: 2019-8753
* Placa utilizada: STM32 NUCLEO-F302R8



Reto asignado

Controlador de iluminación con consola serial.



Descripción

Aplicación bare-metal desarrollada en C para controlar el brillo de un LED mediante PWM de hardware.



El sistema integrará:



* PWM de hardware para control de brillo.
* Comunicación UART a 115200 baudios.
* Pulsador mediante interrupción EXTI.
* Antirrebote no bloqueante.
* Temporizador periódico de 1 ms.
* Máquina de estados MANUAL, AUTO y PAUSA.
* Recepción UART no bloqueante.
* Buffer circular de recepción.
* Parser de comandos.
* Telemetría periódica.
* Manejo y recuperación de errores.



No se utiliza RTOS.



Hardware

\- STM32 NUCLEO-F302R8

\- LED externo

\- Resistencia de 1 kOhm

\- Pulsador B1 integrado en la placa

\- LED LD2 integrado en la placa

\- Cable USB



Software

\- STM32CubeIDE

\- STM32 HAL

\- Git

\- GitHub



Recursos asignados



| Función        | Periférico | Pin/Recurso | Configuración 
                 

| LED de estado  | GPIO       | PB13 / LD2  | Salida digital 

| Pulsador       | GPIO/EXTI  | PC13 / B1   | Interrupción por ambos flancos 

| UART TX        | USART2     | PA2         | 115200 baud 

| UART RX        | USART2     | PA3         | 115200 baud 

| Tick periódico | TIM6       | Interno     | 1 ms 

| PWM            | TIM2 CH1   | PA0         | 1 kHz, duty inicial 25% 





Configuración de temporizadores

TIM6 - Base de tiempo



Reloj efectivo del temporizador: 64 MHz.



\- Prescaler: 63

\- Counter Period: 999



Frecuencia:



64 MHz / ((63 + 1) \* (999 + 1)) = 1000 Hz



Periodo resultante: 1 ms.





TIM2 - PWM



Reloj efectivo del temporizador: 64 MHz.



Configuración inicial:



\- Prescaler: 63

\- Counter Period: 999

\- PWM Channel: TIM2\_CH1

\- Salida: PA0

\- Pulse inicial: 250

\- Frecuencia inicial: 1000 Hz

\- Duty inicial: 25%





Estructura del repositorio



\- `Drivers/`: drivers HAL/CMSIS utilizados por el proyecto.

\- `Startup/`: código de arranque del STM32.

\- `analisis/`: análisis, diagramas y diseño del sistema.

\- `pruebas/`: documentación de las pruebas realizadas.

\- `evidencias/`: capturas y resultados de las pruebas.

\- `2019-8753 Parcial Micro.ioc`: configuración STM32CubeMX/CubeIDE.



El programa principal se encuentra en `Src/main.c`.





Estado actual



Actualmente se configuro inicialmente:



\- GPIO.

\- LED LD2.

\- Pulsador B1.

\- EXTI para el pulsador.

\- USART2.

\- TIM6 para base de tiempo de 1 ms.

\- TIM2 CH1 para PWM por hardware.

\- Antirrebote de 30 ms probado.

\- Pulsador B1 identificado como activo en bajo.

\- Clasificación de pulsaciones cortas implementada y probada.

\- Clasificación de pulsaciones largas de 1500 ms implementada y probada.

\- Protección contra repetición de pulsaciones largas implementada.

\- Contadores independientes de pulsaciones cortas y largas funcionando.


## Configuracion y prueba del PWM

### Objetivo
Configurar y comprobar el funcionamiento del PWM mediante
el temporizador TIM2 del STM32F302R8.

### Configuracion realizada
- Temporizador: TIM2.
- Canal: CH1.
- Pin de salida: PA0.
- Reloj del temporizador: 64 MHz.
- Prescaler: 63.
- Counter Period: 999.
- Pulse: 250.
- Frecuencia calculada: 1000 Hz.
- Duty Cycle inicial: 25%.

### Desarrollo
Se verifico la configuracion de TIM2 en STM32CubeIDE.
Inicialmente, el LED externo no encendia porque el valor
Pulse generado en main.c era 0.

Se corrigio el valor Pulse a 250 desde el archivo .ioc
y se regenero el codigo.

Posteriormente, se compilo el proyecto y se cargo
correctamente en la placa STM32.

### Resultados
- Compilacion exitosa.
- Programacion de la STM32 exitosa.
- LED externo conectado a PA0 encendido.
- Funcionamiento observado compatible con PWM al 25%.
- Frecuencia y duty cycle pendientes de medicion instrumental.

### Estado
Primera prueba del PWM completada correctamente.


### Prueba PWM al 50 %

- Periferico utilizado: TIM2, canal 1.
- Pin de salida: PA0.
- Modo: PWM Mode 1.
- Polaridad: High.
- Prescaler: 63.
- Periodo: 999.
- Pulse: 500.
- Frecuencia calculada: 1000 Hz.
- Duty cycle configurado: 50 %.

Resultados:
- Compilacion completada sin errores.
- Programa cargado correctamente en la STM32.
- LED externo encendido mediante PWM.
- Prueba funcional realizada correctamente.
- Pendiente: medicion instrumental de frecuencia y duty cycle.

Nota: esta configuracion corresponde a una prueba.
El valor inicial definitivo del sistema sera 25 %,
segun los requisitos del parcial.

### Prueba PWM al 75 %

Configuracion:
- Periferico: TIM2.
- Canal: CH1.
- Pin: PA0.
- PWM Mode: 1.
- Polaridad: High.
- Prescaler: 63.
- Counter Period: 999.
- Pulse: 750.
- Frecuencia calculada: 1000 Hz.
- Duty cycle configurado: 75 %.

Resultados:
- Prueba funcional del PWM al 75 %.
- Pendiente: mediciones instrumentales de frecuencia y duty cycle.

Nota:
Esta configuracion es temporal.
El sistema definitivo debe iniciar con duty de 25 %.

### Control dinamico del PWM

Se implemento la funcion PWM_SetDuty() para modificar
el ciclo de trabajo de TIM2 CH1 sin regenerar el codigo.

Caracteristicas:
- Salida PWM: PA0.
- Frecuencia calculada: 1000 Hz.
- Rango de duty: 0 a 100 %.
- Actualizacion mediante __HAL_TIM_SET_COMPARE().
- Configuracion inicial: 25 %.

Pruebas realizadas:
- Compilacion sin errores.
- Prueba funcional de PWM_SetDuty(50).
- LED externo encendido correctamente.
- Restauracion del duty inicial al 25 %.

Pendiente:
- Medicion instrumental de frecuencia y duty.
- Verificacion de los extremos 0 % y 100 %.
- Integracion con comandos UART y maquina de estados.

## Prueba de comunicacion UART - USART2

**Hardware:** STM32 NUCLEO-F302R8

**Configuracion:**
- Puerto serial: COM5
- Velocidad: 115200 baudios
- Formato: 8N1
- Comunicacion mediante ST-LINK Virtual COM Port
- Recepcion UART por interrupciones
- Buffer circular de recepcion de 128 bytes
- Transmision UART por interrupciones

**Procedimiento:**
1. Compilar el proyecto en STM32CubeIDE.
2. Programar la STM32 mediante ST-LINK.
3. Abrir PuTTY utilizando COM5 y 115200 baudios.
4. Enviar el comando HELP y presionar Enter.
5. Verificar la respuesta del microcontrolador.

**Resultado:**
- Compilacion realizada correctamente.
- Programa cargado en la STM32.
- LED externo funcionando.
- Comando HELP recibido y procesado correctamente.
- La STM32 respondio mostrando la lista de comandos.
- Se verifico la comunicacion UART bidireccional.

**Observacion:**
PuTTY inicialmente no mostraba los caracteres escritos
porque el eco local estaba desactivado. Esto no afecto
el funcionamiento de la comunicacion UART.

**Pendiente:**
Implementar y validar los comandos restantes,
la maquina de estados y las pruebas completas.

## Implementacion y prueba del comando STATUS

Se implemento el comando STATUS mediante USART2 para
consultar la configuracion actual del PWM generado por TIM2.

Configuracion:
- Microcontrolador: STM32F302R8.
- Comunicacion UART: 115200 baudios, 8N1.
- Puerto utilizado: COM5.
- Temporizador PWM: TIM2, canal 1, PA0.
- Frecuencia configurada: 1000 Hz.
- Duty cycle inicial: 25%.

Cambios realizados:
- Se agrego la variable pwmDuty para almacenar el duty cycle.
- Se actualizo PWM_SetDuty para guardar el porcentaje configurado.
- Se implemento STATUS utilizando snprintf.
- Se obtiene el prescaler desde el registro PSC de TIM2.
- Se obtiene el periodo desde el registro ARR de TIM2.
- Se calcula la frecuencia del PWM a partir de estos valores.

Prueba realizada:
1. Compilar el proyecto sin errores.
2. Programar la STM32 mediante ST-LINK.
3. Abrir PuTTY en COM5 a 115200 baudios.
4. Enviar STATUS y presionar Enter.

Respuesta obtenida:
OK
MODE: MANUAL
PWM FREQ: 1000 Hz
PWM DUTY: 25%

Resultado: PRUEBA EXITOSA.

Nota: MODE MANUAL es actualmente una respuesta fija.
La maquina de estados sera implementada luego.

## Control del PWM mediante el comando DUTY

Se implemento el comando DUTY n para modificar el ciclo
de trabajo del PWM de TIM2, canal 1, desde la terminal UART.

Configuracion:
- Microcontrolador: STM32F302R8.
- Salida PWM: TIM2_CH1, PA0.
- Frecuencia PWM: 1000 Hz.
- Comunicacion: USART2, 115200 baudios, 8N1.
- Terminal utilizada: PuTTY, COM5.
- Rango de duty cycle: 0 a 100%.

Implementacion:
- Se agrego UART_ParseDuty para validar valores numericos.
- Se incorporo DUTY n al interprete de comandos UART.
- Se utiliza PWM_SetDuty para actualizar el registro CCR1.
- La variable pwmDuty almacena el ultimo porcentaje.
- El comando STATUS permite consultar el duty configurado.

Pruebas realizadas:
- DUTY 25: cambio de brillo correcto.
- DUTY 50: cambio de brillo correcto.
- DUTY 75: cambio de brillo correcto.
- DUTY 0: LED apagado.
- DUTY 100: LED encendido al maximo.
- DUTY 150: valor rechazado.
- STATUS: muestra el ultimo duty configurado.

Resultado: PRUEBA FUNCIONAL EXITOSA.

Pendiente:
- Implementar el comando FREQ n.
- Implementar los modos MANUAL, AUTO y PAUSA.
- Completar las pruebas de integracion.

## Control de frecuencia PWM mediante UART

Se implemento el comando FREQ n para modificar la
frecuencia del PWM generado por TIM2, canal 1, PA0.

Configuracion:
- Microcontrolador: STM32F302R8.
- Temporizador: TIM2.
- Canal PWM: CH1.
- Reloj del temporizador: 64 MHz.
- Prescaler: 63.
- Frecuencias permitidas: 500, 1000 y 2000 Hz.
- Comunicacion: USART2, 115200 baudios.
- Terminal: PuTTY, COM5.

Implementacion:
- Se agrego la funcion PWM_SetFrequency.
- Se implemento UART_ParseFrequency para validar valores.
- Se incorporo el comando FREQ n al interprete UART.
- Se actualiza ARR para modificar el periodo del PWM.
- Se recalcula CCR1 para conservar el duty configurado.
- STATUS permite consultar la frecuencia resultante.

Pruebas funcionales:
- FREQ 500: respuesta OK FREQ.
- STATUS: frecuencia reportada de 500 Hz.
- FREQ 1000: respuesta OK FREQ.
- STATUS: frecuencia reportada de 1000 Hz.
- FREQ 2000: respuesta OK FREQ.
- STATUS: frecuencia reportada de 2000 Hz.
- FREQ 1500: respuesta ERR FREQ.

Resultado:
Pruebas funcionales por UART completadas correctamente.

Pendiente:
- Medir la frecuencia real con osciloscopio
  o analizador logico.
- Verificar transiciones de frecuencia.
- Implementar los modos MANUAL, AUTO y PAUSA.

## Maquina de estados MANUAL, AUTO y PAUSA

Se implemento una maquina de estados para controlar
el funcionamiento del sistema mediante el boton B1.

Estados implementados:
- MANUAL: estado inicial del sistema.
- AUTO: estado seleccionado mediante pulsacion corta.
- PAUSA: suspende temporalmente la salida PWM.

Control mediante boton:
- Pulsacion corta: entre 30 ms y menos de 1500 ms.
- Pulsacion larga: 1500 ms o mas.
- Antirrebote: 30 ms.
- Deteccion mediante EXTI y temporizacion con TIM6.

Transiciones:
- MANUAL -> AUTO: pulsacion corta.
- AUTO -> MANUAL: pulsacion corta.
- MANUAL/AUTO -> PAUSA: pulsacion larga.
- PAUSA -> estado anterior: pulsacion larga.
- Pulsacion corta en PAUSA: no cambia el estado.

Comportamiento de PAUSA:
- Se conserva el estado anterior.
- Se guarda el duty cycle antes de entrar en PAUSA.
- La salida PWM pasa a 0%.
- Al reanudar, se recupera el duty anterior.

Comando STATUS:
- Muestra el estado real del sistema.
- Informa la frecuencia PWM configurada.
- Informa el duty cycle actual.

Pruebas realizadas:
- Cambio MANUAL/AUTO mediante B1: correcto.
- Entrada y salida de PAUSA: correcto.
- PWM al 0% durante PAUSA: correcto.
- Recuperacion del duty anterior: correcto.
- Visualizacion de los tres estados en STATUS: correcto.

Pendiente:
- Implementar la rampa automatica en AUTO.
- Implementar los indicadores LED de cada estado.
- Completar comandos UART de control de estados.
- Realizar pruebas finales con instrumentos.

## Rampa automatica PWM

Se implemento el comportamiento del modo AUTO mediante
TIM2 para generar PWM y TIM6 como base de tiempo de 1 ms.

Caracteristicas:
- Duty minimo: 10%.
- Duty maximo: 90%.
- Incremento o decremento: 1%.
- Intervalo de actualizacion: 20 ms.
- Tiempo nominal de subida: 1.6 segundos.
- Tiempo nominal de bajada: 1.6 segundos.
- Duracion nominal del ciclo completo: 3.2 segundos.
- Funcionamiento no bloqueante.

Funcionamiento:
- Una pulsacion corta cambia de MANUAL a AUTO.
- La rampa inicia en 10% y aumenta hasta 90%.
- Posteriormente disminuye hasta 10%.
- El ciclo se repite continuamente.
- Al regresar a MANUAL se recupera el duty anterior.
- Una pulsacion larga activa PAUSA y coloca PWM en 0%.
- Al salir de PAUSA se recupera la rampa automatica.

Variables utilizadas:
- autoDuty: valor actual de la rampa.
- autoDirection: sentido de incremento o decremento.
- autoLastUpdate: referencia temporal.
- manualDuty: duty conservado del modo MANUAL.

Pruebas funcionales:
- Variacion continua de intensidad en AUTO: correcta.
- Entrada a PAUSA durante AUTO: correcta.
- Recuperacion de AUTO despues de PAUSA: correcta.
- Recuperacion de 75% al regresar a MANUAL: correcta.
- Consulta del estado mediante STATUS: correcta.

Pendiente:
- Verificacion instrumental del periodo de la rampa.
- Completar comandos UART de control de estados.
- Implementar indicadores LED de los modos.
- Completar pruebas de robustez.

## Control de estados mediante UART

Se implementaron comandos para controlar la maquina
de estados desde PuTTY mediante USART2 a 115200 baudios.

Comandos implementados:
- MODE MANUAL: selecciona el funcionamiento manual.
- MODE AUTO: inicia la rampa automatica.
- PAUSE: suspende el funcionamiento y apaga el PWM.
- RESUME: recupera el estado anterior.

Comportamiento:
- MODE AUTO inicia la rampa desde 10%.
- MODE MANUAL recupera el duty manual anterior.
- PAUSE guarda el estado y duty actuales.
- Durante PAUSA, la salida PWM permanece en 0%.
- RESUME recupera el estado y duty anteriores.
- La rampa AUTO continua despues de RESUME.
- Los comandos MODE son rechazados durante PAUSA.

Validaciones:
- PAUSE repetido responde ERR ALREADY PAUSED.
- RESUME fuera de PAUSA responde ERR NOT PAUSED.
- MODE durante PAUSA responde ERR PAUSED.

Pruebas funcionales:
- Cambio MANUAL/AUTO por UART: correcto.
- Recuperacion del duty manual: correcta.
- PAUSE y RESUME en MANUAL: correctos.
- PAUSE y RESUME en AUTO: correctos.
- Validacion de comandos incorrectos: correcta.
- Compatibilidad con el boton B1: correcta.

Pendiente:
- Implementar STREAM ON/OFF.
- Implementar indicadores LED de los estados.
- Validar restricciones de DUTY durante AUTO y PAUSA.
- Realizar pruebas finales y documentar evidencias.

## Transmision periodica UART - STREAM

Se implementaron los comandos STREAM ON y STREAM OFF.

Funcionamiento:
- STREAM ON activa la transmision periodica.
- STREAM OFF detiene la transmision.
- Intervalo nominal: 1000 ms.
- Se transmite el modo, frecuencia y duty PWM.
- El envio utiliza la cola UART no bloqueante.
- La temporizacion utiliza TIM6 mediante g_ms.

Pruebas:
- Recepcion de mensajes periodicos: correcta.
- STREAM OFF durante la transmision: correcto.
- Recepcion de comandos mientras STREAM esta activo: correcta.

Observacion:
En PuTTY, los mensajes periodicos pueden mezclarse
visualmente con los caracteres escritos por el usuario.
Esto no impide que la STM32 procese el comando completo.

Pendiente:
- Verificar STREAM en todos los estados.
- Implementar indicadores LED de estados.
- Completar validaciones y pruebas finales.


## Indicador LED de estados

Se implementó un indicador visual mediante una salida GPIO
configurada en PB13.

El LED permite identificar el modo de funcionamiento:

- MANUAL: permanece encendido continuamente.
- AUTO: cambia de estado cada 500 ms.
- PAUSA: cambia de estado cada 125 ms.

El control del indicador utiliza el contador de milisegundos
del sistema, sin retardos bloqueantes.

### Pruebas realizadas

- Se verificó el encendido y apagado físico del LED.
- Se comprobó el encendido permanente en modo MANUAL.
- Se verificó el parpadeo lento en modo AUTO.
- Se verificó el parpadeo rápido en modo PAUSA.
- Se comprobó el funcionamiento mediante comandos UART.

Resultado: pruebas satisfactorias.

## Protección de comandos según el modo

Se implementó una validación del comando DUTY para evitar
modificaciones no autorizadas del PWM.

- MANUAL: permite configurar el duty entre 0 % y 100 %.
- AUTO: rechaza DUTY con el mensaje ERR AUTO MODE.
- PAUSA: rechaza DUTY con el mensaje ERR PAUSED.

El sistema conserva el valor manual configurado y evita que
un comando DUTY reactive el PWM durante la pausa.

### Pruebas realizadas

- DUTY 75 en MANUAL: aceptado.
- DUTY 50 en AUTO: rechazado; la rampa continúa.
- DUTY 80 en PAUSA: rechazado; PWM permanece en 0 %.
- STATUS durante PAUSA: confirma MODE PAUSA y DUTY 0 %.
- RESUME: recupera el modo de funcionamiento anterior.

Resultado: pruebas satisfactorias.

## Pruebas de robustez UART

Se realizaron pruebas para verificar el manejo de comandos
incorrectos y la recuperación del sistema ante errores.

### Validaciones realizadas

- DUTY 101: rechazado.
- DUTY -100: rechazado.
- DUTY ABC: rechazado.
- FREQ 750: rechazado.
- FREQ 3000: rechazado.
- KLK: comando desconocido, rechazado.
- DUTY 50: aceptado correctamente.
- STATUS: funcionamiento correcto después de los errores.

### Prueba de longitud de línea

Se transmitió una cadena superior al límite de 63 caracteres.

El receptor descartó la línea sin ejecutar comandos
incorrectos y continuó procesando nuevas instrucciones.

Posteriormente se ejecutó STATUS y se obtuvo una
respuesta correcta.

Resultado: todas las pruebas realizadas fueron satisfactorias.
