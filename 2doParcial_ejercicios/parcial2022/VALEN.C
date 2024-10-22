/*
 * Programar el microcontrolador LPC1769 para que mediante su ADC digitalice dos senales
 * analogicas cuyos anchos de banda son de 10KHz cada una. Los canales utilizados deben ser
 * el 2 y 4 y los datos deben ser guardados en dos regiones de memoria distintas que permitan
 * contar con los ultimos 20 datos de cada canal. Suponer una frecuencia de core cclk 100MHz
 *
 */
#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"

#define table 20
uint32_t valores_0[table];
uint32_t valores_1[table];

void config_pins();
void config_ADC();
void config_DMA();

void config_pins(void){
    PINSEL_CFG_Type pinsel_cfg;
    pinsel_cfg.PortNum=0;
    pinsel_cfg.PinNum= 23; //AD0.0
    pinsel_cfg.FuncNum=2;
    pinsel_cfg.PinMode=PINSEL_PINMODE_TRISTATE; //neither
    pinsel_cfg.OpenDrain=PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pinsel_cfg);
    pinsel_cfg.PinNum=24;//AD0.1
    PINSEL_ConfigPin(&pinsel_cfg);
}
void config_ADC(void){
    //Tengo que ponerle una frecuencia de muestreo de 40KHz para que muestree cada canal a 20KHz(El doble de ancho de banda de la señal)
    ADC_Init(LPC_ADC,40000);
    //Configuro en modo burst
    ADC_BurstCmd(LPC_ADC);
    ADC_ChannelCmd(LPC_ADC,0,ENABLE);
    ADC_ChannelCmd(LPC_ADC,1,ENABLE);
    ADC_IntConfig(LPC_ADC,ADINTEN0,ENABLE);
    ADC_IntConfig(LPC_ADC,ADINTEN1,ENABLE);
    ADC_StartCmd(LPC_ADC, CONTINUOUS);
    //Para usar con DMA deshabilito interrupciones de nvic
    NVIC_DisableIRQ(ADC_IRQn);
}

void confDMA(void){
    GPDMA_LLI_Type DMA_LLI_Struct1;
    GPDMA_LLI_Type DMA_LLI_Struct2;
    DMA_LLI_Struct1.SrcAddr= (uint32_t)&(LPC_ADC-> ADDR0)
    DMA_LLI_Struct1.DstAddr= (uint32_t) valores_0;
    DMA_LLI_Struct1.NextLLI=(uint32_t)&(DMA_LLI_Struct1);
    DMA_LLI_Struct1.Control= table
                              |(2<<18) //source widht 32 bits
                              |(2<<18) //destination widht 32 bits
                              |(1<<26) //source increment
                              |(0<<27) //no dest increment
                              ;
    DMA_LLI_Struct2.SrcAddr= (uint32_t)&(LPC_ADC->ADDR1)
    DMA_LLI_Struct2.DstAddr= (uint32_t) valore_1;
    DMA_LLI_Struct2.NextLLI= (uint32_t)&(DMA_LLI_Struct2);
    DMA_LLI_Struct2.Control= table
                              |(2<<18) //source widht 32 bits
                              |(2<<18) //destination widht 32 bits
                              |(1<<26) //source increment
                              |(0<<27) //no dest increment
                              ;
    //Armar el block diagram para cada tabla, cada una va a ocupar un canal distinto
    GPDMA_Init();
    GPDMA_Channel_CFG_Type channel0;
    GPDMA_Channel_CFG_Type channel1;
    channel0.ChannelNum=0;
    channel0.TransferSize= (uint32_t)table;
    channel0.TransferWidth= 0; //solo para M2P
    channel0.SrcMemAddr= (uint32_t)&(LPC_ADC->ADDR0);
    channel0.DstMemAddr= (uint32_t)&valores_0
    channel0.TransferType= GPDMA_TRANSFERTYPE_P2M;
    channel0.SrcConn= GPDMA_CONN_ADC;
    channel0.DstConn= (uint32_t) 0;
    channel0.DMALLI= (uint32_t)&DMA_LLI_Struct1;

    channel1.ChannelNum=1;
    channel1.TransferSize= (uint32_t)table;
    channel1.TransferWidth= 0; //solo para M2P
    channel1.SrcMemAddr= (uint32_t)&(LPC_ADC->ADDR1);
    channel1.DstMemAddr= (uint32_t)&valores_1
    channel1.TransferType= GPDMA_TRANSFERTYPE_P2M;
    channel1.SrcConn= GPDMA_CONN_ADC;
    channel1.DstConn= (uint32_t) 0;
    channel1.DMALLI= (uint32_t)&DMA_LLI_Struct2;

    GPDMA_Setup(&channel0);
    GPDMA_Setup(&channel1);
    GPDMA_ChannelCmd(0,ENABLE);
    GPDMA_ChannelCmd(1,ENABLE);
    return;
}

int main (void){
    confDMA();
    config_ADC();
    config_pin();
    while(1){
        //espero interrupcion
    }
}