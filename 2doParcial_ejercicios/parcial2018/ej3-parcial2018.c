/*
Considerando que se tiene un bloque de datos comprendidos entre las posiciones de memorias, dirección inicial= 
0x10000800 a la dirección final= 0x10001000 ambas inclusive y se desea trasladar este bloque  de datos una nueva 
zona de memoria comprendida entre la dirección inicial= 0x10002800 y la dirección Final=0x10003000 (en el mismo 
orden). Teniendo en cuenta además que los datos contenidos dentro de la zona de  memoria son de 16 bits (AHB 
Master endianness configuration - por defecto) y que estos deben moverse de a uno (1)  en cada evento de DMA, se 
sincronizará la transmisión con evento de match0 del timer1
*/

#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"

void configDMA(void);
void configTimer(void);

int main(void) {
    configDMA();
    while(1){

    }
    return 1;
}

/* calculamos el buffer size para la configuracion
Inicio = 0x10000800
Fin = 0x10001000
bufferSize = (Fin - Inicio) + 2 = 0x0802 direcciones de memoria (osea, bytes)
Como dice el problema, los datos son de 16 bits, entonces el tamaño del buffer expresado en halfwords es de 0x0802 / 2 = 0x0401
*/
void configDMA(void){
    
    GPDMA_Channel_CFG_Type dmaConfig;
    dmaConfig.ChannelNum = 0;
    dmaConfig.TransferSize = 0x0401; 
    dmaConfig.TransferWidth = GPDMA_WIDTH_HALFWORD; //los datos contenidos dentro de la zona de  memoria son de 16 bits
    dmaConfig.SrcMemAddr = (uint32_t *) 0x10000800; //direccion inicial de origen
    dmaConfig.DstMemAddr = (uint32_t *) 0x10002800; //direccion inicial de destino 
    dmaConfig.TransferType = GPDMA_TRANSFERTYPE_M2M;

    GPDMA_Init();
    GPDMA_Setup(&dmaConfig);
    GPDMA_ChannelCmd(0, ENABLE);
}

void configTimer(void) {

    // Configuración del timer1 para generar eventos de match0
    TIM_TIMERCFG_Type timerConfig;
    timerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
    timerConfig.PrescaleValue = 1; // Prescaler para contar microsegundos

    TIM_MATCHCFG_Type matchConfig;
    matchConfig.MatchChannel = 0;
    matchConfig.IntOnMatch = ENABLE; // Habilitar interrupción en match
    matchConfig.ResetOnMatch = ENABLE; // Resetear el timer en match
    matchConfig.StopOnMatch = DISABLE; // No detener el timer en match
    matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchConfig.MatchValue = 1000; // Valor de match aleatorio

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timerConfig);
    TIM_ConfigMatch(LPC_TIM1, &matchConfig);
    TIM_Cmd(LPC_TIM1, ENABLE); // Habilitar el timer

    NVIC_EnableIRQ(TIMER1_IRQn); // Habilitar interrupción del timer1

}

void TIMER1_IRQHandler(void) {
    
    // Generar evento de DMA
    GPDMA_ChannelCmd(0, ENABLE);
    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT); // Limpiar la interrupción del timer
    
}