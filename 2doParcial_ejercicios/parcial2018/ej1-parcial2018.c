/*
Utilizando CMSIS escriba y comente un código que genere una onda del tipo trapezoidal a la salida del DAC como se
muestra en la figura. Para ello el DAC comenzará, a partir de cero, a incrementar su valor de a un bits hasta llegar a un
valor máximo que se mantendrá un tiempo dado y luego decrementará de a un bits hasta volver a cero nuevamente. Los
controles del valor máximo y los tiempos de duración de los escalones de subida y bajada están organizados en la
posición de memoria 0x10004000 de la siguiente forma:
bits 0 a 7: valor máximo a alcanzar por el DAC.
bits 8 a 15: valor a utilizar en una función de demora que define el tiempo que se mantiene el valor máximo.
bits 16 a 23: valor a utilizar en una función de demora para definir el tiempo que se mantiene cada incremento de 1 bits
en la subida.
bits 24 a 31: valor a utilizar en una función de demora para definir el tiempo que se mantiene cada decremento de 1 bits
en bajada.
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

void configDAC(void);
void configT0(void);


int main (void){

  /* Obtenemos los valores importantes
  con (uint32_t *)0x10004000 le indicamos que el nro es una direccion de memoria y con el * externo,
 le indicamos que queremos el contenido de dicha direccion, luego enmascaramos para obtener los valores que queremos */
  uint8_t t_decremento =  (*((uint32_t *)0x10004000) >> 24) & 0xFF;
  uint8_t t_incremento = (*((uint32_t *)0x10004000) >> 16) & 0xFF;
  uint8_t t_maximo = (*((uint32_t *)0x10004000) >> 8) & 0xFF;
  uint8_t valor_max = (*((uint32_t *)0x10004000) >> 0) & 0xFF;

  uint16_t valor_funcion = 0;

  configDAC();
  configT0();
  while(1){

  }
  return 0;
  
}

void configDAC(void){

  //configuracion del pin de salida
  PINSEL_CFG_Type pinAOUT;
  pinAOUT.Portnum = 0;
  pinAOUT.Pinnum = 26;
  pinAOUT.Funcnum = 2;
  PINSEL_ConfigPin(&pinAOUT);

  DAC_CONVERTER_CFG_Type configDAC;
  configDAC.DBLBUF_ENA = ENABLE; //Para generar formas de onda se recomienda seleccionarlo
  configDAC.CNT_ENA = DISABLE;
  configDAC.DMA_ENA = DISABLE;

  DAC_Init(LPC_DAC);
  DAC_ConfigDAConverterControl(LPC_ADC, &configDAC);
  DAC_UpdateValue(valor_funcion);


}

void configT0(void){

  TIM_TIMERCFG_Type configt0;
  configT0.PrescaleOption = TIM_PRESCALE_USVAL;
  //Suponiendo que los t guardados en 0x10004000 son en milisegundos
  configT0.PrescaleValue = 1000; 

  TIM_MATCHCFG_Type match0;
  match0.MatchChannel = 0;
  match0.IntOnMatch = ENABLE;
  match0.StopOnMatch = DISABLE;
  match0.ResetOnMatch = ENABLE;
  match0.MatchValue = t_incremento;

  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &configt0);
  TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
  TIM_ConfigMatch(LPC_TIM0, &match0);
  TIM_Cmd(LPC_TIM0, ENABLE)
  NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(void){
  TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

  switch(match0.MatchValue):
    case t_incremento:
        //Proximo escalon
        valor_funcion++;
        if(valor_funcion == valor_max){
          //Me mantengo
          TIM_UpdateMatchValue(LPC_TIM0, 0, t_maximo);
        } 
        break;
    case t_maximo:
        //Si finalizo el tiempo en el que se queda en el max, empiezo a bajar
        valor_funcion--;
        TIM_UpdateMatchValue(LPC_TIM0, 0, t_decremento);
        break;
    case t_decremento:
        valor_funcion--;
        if(valor_funcion == 0){
          //Si llego al cero vuelvo a subir
          TIM_UpdateMatchValue(LPC_TIM0, 0, t_incremento);
        } 
        break;
  DAC_UpdateValue(valor_funcion);

}
