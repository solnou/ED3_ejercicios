## Definir la funcion del bit BIAS del FAC

El BIAS es un bit del registro DACR  para cambiar el rendimiento del conversor. Se lo puede poner en dos modos:
- `0` : configura al DAC para convertir mas rapido. El consumo de corriente es de 700uA, haciendo que el tiempo de reaccion o setting del DAC sea de 1us, dando una tasa de actualizacion del DAC maxima de 1Mhz

- `1` : configuracion del DAC en bajo consumo. El consumo de corriente es menor que el anterior, de 350uA, por lo que el tiempo de reaccion se relentiza a 2.5us, dando una tasa de actualizacion maxima de 400KHz

- Dependiendo de la aplicación, puedes ajustar este bit para equilibrar la velocidad de conversión y el consumo de energía, según sea necesario
