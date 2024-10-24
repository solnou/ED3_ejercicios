/* 
Utilizando el timer0, un dac, interrupciones y el driver del LPC1769 , escribir un código que 
permita  generar  una  señal  triangular  periódica  simétrica,  que  tenga  el  mínimo  periodo 
posible, la máxima excursión de voltaje pico a pico posible y el mínimo incremento de señal 
posible por el dac. Suponer una frecuencia de core cclk de 100 Mhz. El código debe estar 
debidamente comentado
*/
#include "lpc17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

#define DAC_VALUE_MAX 1024 // Valor máximo para un DAC de 10 bits
volatile uint32_t valor_señal = 512; // Variable que va dando el valor de la señal a convertir por el DAC

void configDAC(void);
void configTimer(void);

int main (void) {
    configDAC();
    configTimer();
    while(1) {
        
    }
    return 1;
}

void configDAC(void) {

    // Configurar el pin por donde sale la señal del DAC, P0.26
    PINSEL_CFG_Type pinConfig;
    pinConfig.Funcnum = 2; // AOUT
    pinConfig.Pinnum = 26;
    pinConfig.Portnum = 0;
    PINSEL_ConfigPin(&pinConfig);

    DAC_CONVERTER_CFG_Type configDAC;
    configDAC.DBLBUF_ENA = DISABLE;
    configDAC.CNT_ENA = DISABLE;
    configDAC.DMA_ENA = DISABLE;

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &configDAC);
    DAC_SetBias(LPC_DAC, 0); // Más rápido
    DAC_UpdateValue(LPC_DAC, valor_señal); // Empieza en la mitad del rango, para arrancar desde el origen, el punto medio
}

void configTimer(void) {
    
    // Para el mínimo periodo posible el timer tiene que ser lo más rápido posible
    TIM_TIMERCFG_Type timerConfig;
    timerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
    timerConfig.PrescaleValue = 0; // Para mayor resolución sacamos el PR

    TIM_MATCHCFG_Type matchConfig;
    matchConfig.MatchChannel = 0;
    matchConfig.IntOnMatch = ENABLE; // Para modificar el DAC periódicamente
    matchConfig.ResetOnMatch = ENABLE;
    matchConfig.StopOnMatch = DISABLE;
    matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchConfig.MatchValue = 1; // El mínimo valor
    
    TIM_ConfigMatch(LPC_TIMER0, &matchConfig); // Configuramos el match
    TIM_Init(LPC_TIMER0, TIM_TIMER_MODE, &timerConfig); // Inicializamos el timer
    TIM_ClearIntPending(LPC_TIMER0, TIM_MR0_INT); // Limpiamos las flags
    
    NVIC_EnableIRQ(TIMER0_IRQn); // Habilitamos la interrupción
    TIM_Cmd(LPC_TIMER0, ENABLE); // Habilitamos el timer
}

void TIMER0_IRQHandler(void) {
    
   // Generar señal triangular
    if (valor_señal >= DAC_VALUE_MAX) {
        paso = -1; // Cambiar a decrementar
    } else if (valor_señal <= 0) {
        paso = 1; // Cambiar a incrementar
    }

    valor_señal += paso; // Incrementar o decrementar según el valor de increment
   
    DAC_UpdateValue(LPC_DAC, valor_señal); // Actualizamos el valor del DAC
    TIM_ClearIntPending(LPC_TIMER0, TIM_MR0_INT); // Limpiamos la interrupción del timer
}
