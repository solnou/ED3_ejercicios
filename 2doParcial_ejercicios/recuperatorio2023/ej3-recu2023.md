## Describir las formas de disparo o de inicio de conversion del ADC de LPC1769

- `ADC_START_CONTINUOS` o `BURST`: Este modo es una ráfaga de conversión del ADC. Hace conversiones continuas a 200KHz. Es útil para aplicaciones que requieren monitoreo constante de señales analógicas.
- `ADC_START_NOW`: ealiza una única conversión en el momento que se setea esta función en LPC_ADC->AD0CR[26:24]. Es para tiempos de conversion precisos o conversiones de baja demanda
- `ADC_START_ON_EINTn`:  Inicia la conversión cuando por el pin EINTn ocurre el flanco sensible seleccionado en su configuración. Este modo es útil para aplicaciones que requieren conversión basada en eventos externos, como interrupciones.
- `ADC_START_ON_CAPn`: Inicia la conversión cuando por el pin CAPn.n ocurre el flanco sensible seleccionado en la configuración del timer. Este modo es útil para aplicaciones que requieren sincronización precisa con eventos de captura de temporizador.
- `ADC_START_ON_MATn`: Inicia la conversión cuando por el pin MATn.n ocurre el flanco seleccionado en la configuración del timer. Este modo es útil para aplicaciones que requieren sincronización con eventos de coincidencia de temporizador.
