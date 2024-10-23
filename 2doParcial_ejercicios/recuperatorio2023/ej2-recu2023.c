/*
Se tienen tres bloques de datos de 4KBytes de longitud cada uno en el cual se han guardado tres formas 
de onda. Cada muestra de la onda es un valor de 32 bits que se ha capturado desde el ADC. Las direcciones de 
inicio de cada bloque son representadas por macros del estilo DIRECCION_BLOQUE_N, con N=0,1,2.
Se pide que, usando DMA y DAC se genere una forma de onda por la salida analógica de la LPC1769. 
La forma de onda cambiará en función de una interrupción externa conectada a la placa de la siguiente 
manera:
● 1er interrupción: Forma de onda almacenada en bloque 0, con frecuencia de señal de 60[KHz].
● 2da interrupción: Forma de onda almacenada en bloque 1 con frecuencia de señal de 120[KHz].
● 3ra interrupción: Forma de onda almacenada en bloque 0 y bloque 2 (una a continuación de la otra) 
con frecuencia de señal de 450[KHz].
● 4ta interrupción: Vuelve a comenzar con la forma de onda del bloque 0.
En cada caso se debe utilizar el menor consumo de energía posible del DAC.
__________________________________________________________________________

*/

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_adc.h"

#define BLOCK_SIZE 1024
GPDMA_Channel_CFG_Type configDMA
GPDMA_LLI_Type LLI0


void configEINT(void);
void configDAC(void);
void configDMA(void);
uint16_t calcular_timeout(uint16_t frecuencia, uint16_t BLOCK_SIZE);

int main(void){

    configEINT();
    configDAC();
    configDMA();
    uint8_t contador = 1;
    while(1){

    }
    return 0;
}

void configEINT(void){

    PINSEL_CFG_Type pin_cfg; 
    pin_cfg.Portnum = PINSEL_PORT_2;    
    pin_cfg.Pinnum = PINSEL_PIN_10;            
    pin_cfg.Funcnum = PINSEL_FUNC_1;          
    PINSEL_ConfigPin(&pin_cfg);

    //Configurar el pin
    EXTI_InitTypeDef configEINT;
    configEINT.EXTI_Line = EXTI_EINT0;
    configEINT.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    configEINT.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
    EXTI_Init();
    EXTI_Config(&configEINT);
    EXTI_ClearEXTIFlag(EXTI_EINT0);

    NVIC_EnableIRQ(EINT0_IRQn);

}

void configDAC(void){

    //Configurar el pin de salida P0.26
    PINSEL_CFG_Type configDAC;
    configDAC.Portnum = 0;
    configDAC.Pinnum = 26;
    configDAC.Funcnum = 2;
    PINSEL_ConfigPin(&configDAC);

    //Configurar el DAC
    DAC_CONVERTER_CFG_Type configDAC;
    configDAC.CNT_ENA = ENABLE;
    configDAC.DBLBUF_ENA = DISABLE;
    configDAC.DMA_ENA = ENABLE;

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &configDAC);
    DAC_SetBias(LPC_DAC, 1); //bajo consumo
    

}


void configDMA(void){

    //Aca solo van las configuraciones generales del DMA, lo que no es comun, se pone ante una interrupcion

    //Primer bloque de memoria
    GPDMA_LLI_Type LLI0;
    LLI0.SrcAddr = DIRECCION_BLOQUE_0;
    LLI0.DstAddr = (uint32_t) & (LPC_DAC->DACR);;
    LLI0.NextLLI = &LLI1;
    LLI.Control = BLOCK_SIZE 
                | (GPDMA_WIDTH_WORD << 18) //Transferencia de 32 bits a DAC(10bits)
                | (GPDMA_WIDTH_WORD << 21) 
                | (1 << 26) //incremento del origen

    //Segundo bloque de memoria
    GPDMA_LLI_Type LLI1;
    LLI1.SrcAddr = DIRECCION_BLOQUE_2;
    LLI1.DstAddr = (uint32_t) & (LPC_DAC->DACR);
    LLI1.NextLLI = 0;
    LLI.Control = BLOCK_SIZE 
                | (GPDMA_WIDTH_WORD << 18) //Transferencia de 32 bits a DAC(10bits)
                | (GPDMA_WIDTH_WORD << 21) 
                | (1 << 26) //incremento del origen

    //Configuracion del canal 0
    GPDMA_Channel_CFG_Type configDMA;
    configDMA.ChannelNum = 0;
    configDMA.TransferSize = BLOCK_SIZE;
    //configDMA.SrcMemAddr es particualr de cada bloque
    configDMA.TransferType = GPDMA_TRANSFERTYPE_M2P; //no debo poner ni width ni dstmemaddr por ser M2P
    configDMA.DstConn = GPDMA_CONN_DAC; 
    //configDMA.DMALLI = es particular de cada bloque

    GPDMA_Init();
    GPDMA_ChannelCmd(0, ENABLE);
}

void EINT0_IRQHandler(void){

    EINT_ClearEXTIFlag(EXTI_EINT0);

    
    switch(contador){
        case 1: //Muestra el bloque 0
            configDMA.SrcMemAddr = DIRECCION_BLOQUE_0;
            configDMA.DMALLI = 0; //solo el primer bloque
            DAC_SetDMATimeOut(calcular_timeout(60000, BLOCK_SIZE));
            GPDMA_Setup(&configDMA, &configDMA);
            break;
        case 2: //Muestra bloque 1
            configDMA.SrcMemAddr = DIRECCION_BLOQUE_1;
            configDMA.DMALLI = 0; //solo el segundo bloque
            DAC_SetDMATimeOut(LPC_DAC, calcular_timeout(120000, BLOCK_SIZE));
            GPDMA_Setup(&configDMA, &configDMA);
            break;
        case 3: //Muestra el bloque y 2 como una sola señal
            configDMA.SrcMemAddr = DIRECCION_BLOQUE_0;
            configDMA.DMALLI = &LLI0; //muestra dos bloques
            DAC_SetDMATimeOut(LPC_DAC, calcular_timeout(450000, 2*BLOCK_SIZE));
            GPDMA_Setup(&configDMA, &configDMA);
            contador = 0;
            break;
        
    }

    contador++;
    

}

uint16_t calcular_timeout(uint16_t frecuencia, uint16_t BLOCK_SIZE){
     //update_interval = (CLOCK_DAC_MHZ * 1000000) / (WAVE_FREQUENCY * NUM_SAMPLES);
     return time_out = (25 * 1000000)/(BLOCK_SIZE*frecuencia);

}
