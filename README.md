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
