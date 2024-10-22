## Ejercicio 3
En la siguiente sección de código se realiza la configuración de un canal de DMA
### Explique detalladamente cómo queda configurado el canal, que tipo de transferencia está realizando.

El canal queda configurado para hacer transferencias de memoria al periferico DAC, para que se conviertan esos datos almacenados en memoria directamente por el DAC sin intervencion del CPU.
Esta configurado para usar el canal 0 del DMA, por donde va a transferir datos de dos lugares distintos de memoria, que los asigna a traves de dos LLI. Entonces, el canal queda configurado por dos listas linkeadas.
Sin embargo, esta mal configurado porque en el LLI0, no se ha linkeado bien al LLI1, si no se a linkeado de nuevo a LLI0. Es decir, se va a estar siempre recorriendo datos_1_global, sin transferir los datos_2_global que estan en la segunda lista linkeada.


### ¿Qué datos se transfieren, de qué posición a cuál y cuántas veces?

Se transfieren los datos de dos bloques de memoria, llamados datos_n_global (estan distribuidos en dos lugares de memoria discontinuos) y el destino es el DACR, es decir directamente para convertir.
El ancho de los datos de LLI es de WORD, es decir se transfieren datos de 32 bits. En el channel el width se ignora ya que solo se tiene en cuenta cuando son transferencias de M2M.
Ambos LLIs tienen el valor TOTAL_SAMPLES en el campo Control, lo que indica que cada LLI está configurado para transferir TOTAL_SAMPLES elementos. Pareciera que la cantidad de datos que se buscan transferir son TOTAL_SAMPLES * 2.
Pero, como dijimos anteriormente, la lista LLI0 esta linkeada a si misma, por lo que se van a estar transfiriendo todos los datos de ese bloque de memoria en bucle infinito, sin poder determinar cuantas veces se va a transferir los datos de esta unica lista (los datos_1_global).

### ¿Cómo  se  define  el  tiempo  de  "Interrup DMA  request"   o  el  tiempo  de  transferencia  de c/dato?

El tiempo de interrupcion o de transferencia dependen de la configuracion del DAC, de su tasa de conversion y el CCLK del sistema. No es algo que se configure o se pueda saber solo con este apartado de codigo.
![image](https://github.com/user-attachments/assets/1aa1c2a6-6014-4f32-84dd-2ece1d06b73d)
El timeout va en el cntval. Cuando llega a ese punto hace la conversion del DAC. Esto se setea con funciones de cmsis de DAC.


