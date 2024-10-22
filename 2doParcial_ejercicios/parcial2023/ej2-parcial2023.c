//NO ESTA TERMINADO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_pinsel.h"



bool normal;

#define TOP_ADDRESS 0x501FFFFF;

#define MIDDLE_ADDRESS 0x500FFFFF; 

#define BOTTOM_ADDRESS 0x50000000;  


void configADC(void);
void configDAC(void);
void configEINT(void);
void configGPDMA(void);


int main (void){

    configADC();
    configDAC();
    configEINT();
    configGPDMA();

    bool normal = true;
 

    while(1){

        if(normal){

            ADC_Init(LPC_ADC, 32000);
            GPDMA_Channel_Cmd(0, ENABLE);               // ADC a Memoria
            GPDMA_Channel_Cmd(1, ENABLE);              // Memoria Top (wave_form) a DAC
            GPDMA_Channel_Cmd(2, DISABLE);              // Memoria Bottom (Datos ADC) a DAC


        }
        else{
            ADC_DeInit(LPC_ADC);
            GPDMA_Channel_Cmd(0, DISABLE);              // ADC a Memoria
            GPDMA_Channel_Cmd(1, DISABLE);              // Memoria Top (wave_form) a DAC
            GPDMA_Channel_Cmd(2, ENABLE);               // Memoria Bottom (Datos ADC) a DAC
        }


    }


void configADC(void){

    PINSEL_CFG_Type pinADC;
    pinADC.Portnum= 0;
    pinADC.Pinnum = 2;
    pinADC.Funcnum = 2;
    PINSEL_ConfigPin(&pinADC);

    ADC_Init(LPC_ADC, 32000);
    ADC_BurstCmd (LPC_ADC, ENABLE); 
    ADC_PowerdownCmd (LPC_ADC, ENABLE); 
    ADC_StartCmd(ADC_START_CONTINUOS);
    ADC_ChannelCMD(LPC_ADC, 7, ENABLE);

    

}

void configDAC(void){

    PINSEL_CFG_Type pinDAC;
    pinDAC.Portnum= 0;
    pinDAC.Pinnum = 26;
    pinDAC.Funcnum = 2;
    PINSEL_ConfigPin(&pinDAC);

    DAC_CONVERTER_CFG_Type configDAC;
    configDAC.DBLBUF_ENA = ENABLE;
    configDAC.CNT_ENA = ENABLE;
    configDAC.DMA_ENA = ENABLE;

    DAC_Init(LPC_DAC);
    DAC_SetBias(LPC_DAC, 0);
    uint32_t timeout = 80000000/(1/0.000614) - 1;
    DAC_SetDMATimeOut(LPC_DAC, timeout); //la mitad del periodo por algo ....
    DAC_ConfigDAConverterControl(&configDAC);


};
void configEINT(void){
    
    //P2.10 as EINT0
	PINSEL_CFG_Type pinCfg;
	pinCfg.Portnum = PINSEL_PORT_2;
	pinCfg.Pinnum = PINSEL_PIN_10;
	pinCfg.Funcnum = PINSEL_FUNC_1;
	pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pinCfg);

    //EINT0 configuration
	EXTI_InitTypeDef extiCfg;
	extiCfg.EXTI_Line = EXTI_EINT0;
	extiCfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	extiCfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&extiCfg);
	EXTI_ClearEXTIFlag(EXTI_EINT0);

    NVIC_EnableIRQ(EINT0_IRQn); 
    

}


void configGPDMA(void){

    //Canal 0 ADC a Memoria
    GPDMA_LLI_Type LLI0;
    LLI0.SrcAddr = (uint32_t) & (LPC_ADC-> AD0DR7) ;
    LLI0.DstAddr = (int *) BOTTOM_ADDRESS;
    LLI0.NextLLI = 0;
    LLI0.Control = MIDDLE_ADDRESS - BOTTOM_ADDRESS

    //Canal 1 Memoria Top (wave_form) a DAC

    //Canal 2 Memoria Bottom (Datos ADC) a DAC



}








    return 1;

}
