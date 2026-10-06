# **Prueba 01 - Pulsador e interrupciones**



## **Objetivo**



Comprobar la detección del pulsador B1, el antirrebote y la clasificación

de pulsaciones cortas y largas.





### **Hardware**



\- Placa: STM32 NUCLEO-F302R8

\- Pulsador: B1 / PC13

\- Interrupción: EXTI13

\- Base de tiempo: TIM6, 1 ms





### **Prueba de pulsación corta**



Procedimiento:



1\. Ejecutar el firmware.

2\. Presionar B1 aproximadamente 500 ms.

3\. Liberar B1.

4\. Revisar las variables mediante el debugger.



Resultado observado:



\- buttonStableState = 1

\- buttonRawState = 1

\- buttonPressed = 0

\- shortPressCount = 1

\- longPressCount = 0

\- longPressReported = 0





Resultado: APROBADO.





### **Prueba de pulsación larga**



Procedimiento:



1\. Mantener B1 presionado durante aproximadamente 3 segundos.

2\. Liberar B1.

3\. Revisar los contadores mediante el debugger.



Resultado observado:



\- La pulsación larga incrementó longPressCount una sola vez.

\- shortPressCount no aumentó al liberar el pulsador.

\- Mantener presionado B1 no generó repeticiones.



Resultado: APROBADO.





### **Conclusión**



La interrupción GPIO, el antirrebote de 30 ms y la clasificación de

pulsaciones funcionan correctamente sin utilizar retardos bloqueantes.

