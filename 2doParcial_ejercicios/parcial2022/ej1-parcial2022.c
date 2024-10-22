/*
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice  dos señales 
analógicas cuyos anchos de bandas son de 10 Khz cada una. Los canales utilizados deben ser 
el  2  y  el  4  y  los  datos  deben  ser  guardados  en  dos  regiones  de  memorias  distintas  que 
permitan contar con los últimos 20 datos de cada canal. Suponer una frecuencia de core cclk 
de 100 Mhz. El código debe estar debidamente comentado.
*/

#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

#define destino_canal1 0x10000000
#define destino_canal2 0x10003FFF
#define block_size 20
uint16_t buffer_canal2[block_size];
uint16_t buffer_canal4[block_size];

void configADC(void);
void configT0(void);

int main(void) {
    configADC();
    configT0();
    while (1) {
    }
}

void configADC(void) {
    // Configuración del pin para el Canal 2
    PINSEL_CFG_Type pinC2config;
    pinC2config.Portnum = 0;
    pinC2config.Pinnum = 25;
    pinC2config.Funcnum = 1;    
    PINSEL_ConfigPin(&pinC2config);

    // Configuración del pin para el Canal 4
    PINSEL_CFG_Type pinC4config;
    pinC4config.Portnum = 1;
    pinC4config.Pinnum = 30;
    pinC4config.Funcnum = 3;    
    PINSEL_ConfigPin(&pinC4config);

    /// Inicialización del ADC
    rate = 100000 * 2 * 2; //ancho de banda * 2 canales * 2 para el teorema Nyquist
    ADC_Init(LPC_ADC, 400000);
    ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
    ADC_ChannelCmd(LPC_ADC, 4, ENABLE);
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);

}

void configT0(void){
    // Configuración del temporizador
    TIM_TIMERCFG_Type timerconfig;
    timerconfig.PrescaleOption = TIM_PRESCALE_USVAL;
    //Necesito que TC se incremente cada 1us, como ya pusimos el prescale en usval no hay que poner PR
    timerconfig.PrescaleValue = 0; // Incremento cada 1us

    // Configuración del match para generar interrupciones a 40 kHz
    TIM_MATCHCFG_Type matchconfig;
    matchconfig.MatchChannel = 0;
    matchconfig.IntOnMatch = ENABLE; //interrumpe para volver a inicializar el adc
    matchconfig.StopOnMatch = DISABLE; //no se detiene para la proxima conversion
    matchconfig.ResetOnMatch = ENABLE; //resetea el TC
    matchconfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchconfig.MatchValue = 25; // 1 / (40 kHz) = 25 us

    TIM_ConfigMatch(LPC_TIM0, &matchconfig);
    // Habilitar la interrupción del temporizador
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);


}

void TIMER0_IRQHandler(void) {

    //Ante el tiempo de muestreo, reviso el estado de la conversion
    if (ADC_ChannelGetStatus(LPC_ADC, 2, ADC_DATA_DONE)==SET){
        //Guardo el valor convertido
        uint16_t valor = ADC_ChannelGetData(LPC_ADC, 2);
        if (buffer1_index < block_size){
            buffer_canal2[buffer2_index] = valor;
            buffer2_index++;        
        }
        else{ //reinicio el buffer circular
            buffer2_index = 0;
            buffer_canal2[buffer2_index] = valor;
            buffer2_index++;
        }

    }
    //Reviso el otro canal
    if (ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)==SET){
        //Guardo el valor convertido
        uint16_t valor = ADC_ChannelGetData(LPC_ADC, 4);
        if (buffer1_index < block_size){
            buffer_canal4[buffer4_index] = valor;
            buffer4_index++;        
        }
        else{ //reinicio el buffer circular
            buffer4_index = 0;
            buffer_canal2[buffer4_index] = valor;
            buffer4_index++;
        }

    }

    // Limpiar la interrupción del temporizador
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    // Iniciar la proxima conversión ADC
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);

}