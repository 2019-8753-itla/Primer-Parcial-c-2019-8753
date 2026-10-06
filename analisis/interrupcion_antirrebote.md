Clasificación de pulsaciones



Una vez validado el estado del pulsador mediante el antirrebote de 30 ms,

se mide la duración de la pulsación utilizando la base de tiempo g\_ms.



El pulsador B1 de la NUCLEO-F302R8 fue comprobado como activo en bajo:



\- GPIO\_PIN\_RESET: pulsador presionado.

\- GPIO\_PIN\_SET: pulsador liberado.





Pulsación corta



Una pulsación se considera corta cuando su duración es menor de 1500 ms.

La acción se procesa al detectar la liberación validada del pulsador.



El contador shortPressCount registra las pulsaciones cortas aceptadas.





Pulsación larga



La pulsación larga se detecta al alcanzar 1500 ms mientras el botón

continúa presionado.



La variable longPressReported evita que mantener presionado el botón

genere múltiples eventos.



Después de detectar una pulsación larga, la liberación del botón no

produce una pulsación corta.



El contador longPressCount registra las pulsaciones largas aceptadas.




Temporización



La duración se calcula mediante diferencias de enteros sin signo:



(uint32\_t)(now - pressStart)



No se utilizan retardos activos ni se reinicia el contador g\_ms.

