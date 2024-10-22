/*
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice  dos señales 
analógicas cuyos anchos de bandas son de 10 Khz cada una. Los canales utilizados deben ser 
el  2  y  el  4  y  los  datos  deben  ser  guardados  en  dos  regiones  de  memorias  distintas  que 
permitan contar con los últimos 20 datos de cada canal. Suponer una frecuencia de core cclk 
de 100 Mhz. El código debe estar debidamente comentado.
*/

#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

#define destino_canal1 0x10000000
#define destino_canal2 0x10003FFF
#define block_size 20

void configADC(void);
void configDMA(void);
void configT0(void);

int main(void) {
    configADC();
    configDMA();
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

void configDMA(void) {

    GPDMA_Init();

    //LLI para ubicar en la primera seccion de memoria, para el canal 2 del ADC
    GPDMA_LLI_Type LLI;
    LLI.SrcAddr = (uint32_t) & (LPC_ADC->ADDR2); //obtengo los datos del canal 2 del adc
    LLI.DstAddr = destino_canal1; //los paso por el canal 1 del dma
    LLI.NextLLI = &LLI; //se autoenlaza para hacer buffer circular
    LLI.Control = block_size << 0  //tamaño de transferencia
                | GPDMA_WIDTH_HALFWORD << 21//ancho del destino
                | 1 << 26 //incremento del origen
                | 1 << 27; //incremento del destino

    GPDMA_Channel_CFG_Type canalConfig;
    canalConfig.ChannelNum = 1;
    canalConfig.TransferSize = block_size;
    canalConfig.TransferWidth = GPDMA_WIDTH_HALFWORD; //el adc convierte 12 bits, lo mas cercano  es16
    canalConfig.DstMemAddr = destino_canal1;
    canalConfig.TransferType = GPDMA_TRANSFERTYPE_P2M;
    canalConfig.SrcConn = GPDMA_CONN_ADC;
    canalConfig.DMALLI = (uint32_t)&LLI;
    //configuramos el priemr canal
    GPDMA_Setup(&canalConfig);

    //modifico la estructura para el canal 2 y LLI2, asi uso la misma y no repito tanto codigo
    LLI.SrcAddr = (uint32_t) & (LPC_ADC->ADDR4); //obtengo los datos del canal4 del adc
    LLI.DstAddr = destino_canal2; //los paso por el canal 1 del dma
    canalConfig.ChannelNum = 2; //cambiamos el canal 
    canalConfig.DstMemAddr = destino_canal2; //cambiamos el destino
    canalConfig.TransferSize = block_size;
    //configuramos el segundo 
    GPDMA_Setup(&canalConfig);

    GPDMA_ChannelCmd(1, ENABLE);
    GPDMA_ChannelCmd(2, ENABLE);
}

void TIMER0_IRQHandler(void) {
    // Limpiar la interrupción del temporizador
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    // Iniciar la proxima conversión ADC
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);
}