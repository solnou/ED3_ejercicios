#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#define BUFFER_SIZE 4
#define indice 0

void configT0(void); //para contar los 30 segundos entre muestras del ADC
void configT1(void); //para contar los 2 min antes de promediar los valores
void configT2(void); //para generar el PWM
void configADC(void);

int main(void) {

    configT0();
    configT1();
    configADC();
    bool voltaje_definido = false;

    while (1) {
        //si ya pasaron los 2 minutos, vemos que hacer con el output
        if (voltaje_definido==true){
            if(voltaje>1){
                if(voltaje<2){
                    //Si el voltaje esta entre 1 y 2V, calculamos cual es el dutty 

                    dutty_promedio = 50 + (voltaje - 1) * 40; // el porcentaje del duty cycle mapiado
                }
                else{
                    //Si el voltaje es mayor a 2V ponemos 3.3V por la salida
                    dutty_promedio = 100; //Para que cuando calcule el tiempo en alto, le de 0.00005

                }
            }
            else{
                //Si el voltaje es menor a 1 ponemos 0V por la salida
                dutty_promedio = 0; //Para que cuando calcule el tiempo en alto, le de 0
            }


            configT2();
        }
        
    }
}

void configT0(void) { 

    //configuramos el timer 0 en modo match, para 30s
    TIM_TIMERCFG_Type timer0;
    timer0.PrescaleOption = TIM_PRESCALE_USVAL;
    timer0.PrescaleValue = 1000000; //se cuenta cada 1 segundo

    TIM_MATCHCFG_Type match0;
    match0.MatchChannel = 0;
    match0.IntOnMatch = ENABLE; //que interrumpa para indicar al adc que debe convertir
    match0.ResetOnMatch = ENABLE; //que vuelva el TC a cero para volver a contar
    match0.StopOnMatch = DISABLE; //que no se detenga el timer
    match0.MatchValue = 30; //interrumpe cada 30 segundos

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer0);
    TIM_ConfigMatch(LPC_TIM0, &match0);
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    TIM_Cmd(LPC_TIM0, ENABLE);

    NVIC_EnableIRQ(TIMER0_IRQn);

}

void configT0(void) { 

    //configuramos el timer 1 en modo match, para 2 min
    TIM_TIMERCFG_Type timer1;
    timer1.PrescaleOption = TIM_PRESCALE_USVAL;
    timer1.PrescaleValue = 1000000; //se cuenta cada 1 segundo

    TIM_MATCHCFG_Type match1;
    match1.MatchChannel = 1;
    match1.IntOnMatch = ENABLE; //que interrumpa para promediar el buffer del ADC
    match1.ResetOnMatch = ENABLE; //que vuelva el TC a cero para volver a contar
    match1.StopOnMatch = DISABLE; //que no se detenga el timer
    match1.MatchValue = 120; //interrumpe cada 2 min, 120 segundos

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer1);
    TIM_ConfigMatch(LPC_TIM1, &match1);
    TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
    TIM_Cmd(LPC_TIM1, ENABLE);

    NVIC_EnableIRQ(TIMER1_IRQn);

}

void configT2(void) { 

    //Configurar el pin de salida match 2.2
    PINSEL_CFG_Type pin_config;
    pin_config.Portnum = 0;
    pin_config.Pinnum = 8;
    pin_config.Funcnum = 3;
    pin_config.Pinmode = 0;
    PINSEL_ConfigPin(&pin_config);
    
    // Configuramos el timer 2 en modo match
    TIM_TIMERCFG_Type timer2;
    timer2.PrescaleOption = TIM_PRESCALE_USVAL;
    timer2.PrescaleValue = 1000000; // Se cuenta cada 1 segundo

    TIM_MATCHCFG_Type match2;
    match2.MatchChannel = 2;
    match2.IntOnMatch = ENABLE; // Que interrumpa para indicar al ADC que debe convertir
    match2.ResetOnMatch = ENABLE; // Que vuelva el TC a cero para volver a contar
    match2.StopOnMatch = ENABLE; // Que no se detenga el timer
    
    uint8_t t_alto_PWM = dutty_promedio * 0.00005 / 100; // Calculamos que tiempo va a estar en alto
    uint8_t t_bajo_PWM = 0.00005 - t_alto_PWM; // Calculamos que tiempo va a estar en bajo

    // Arranca en alto, el match corresponde hasta que momento debe mantenerse asi
    match2.MatchValue = t_alto_PWM;
    match2.ExtMatchOutputType = TIM_EXTMATCH_LOW; //Al terminar el tiempo, ponemos en bajo la señal

    TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer2);
    TIM_ConfigMatch(LPC_TIM2, &match2);
    TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
    TIM_Cmd(LPC_TIM2, ENABLE);

    NVIC_EnableIRQ(TIMER2_IRQn);
}

void TIMER0_IRQHandler(void) {

    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    
    //verificamos que haya terminado la conversion
    if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE) == SET){

        if(indice< BUFFER_SIZE){
            //Guardamos la conversion
            buffer_ADC[indice] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
            indice ++;

            //Iniciamos la proxima conversion
            ADC_StartCmd(LPC_ADC, ADC_START_NOW);
        }
        else{
            indice = 0;
            //Guardamos la conversion
            buffer_ADC[indice] = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
            indice ++;

            //Iniciamos la proxima conversion
            ADC_StartCmd(LPC_ADC, ADC_START_NOW);
        }
    }
}

void TIMER1_IRQHandler(void){

    TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);

    //Promediamos los valores del buffer
    for (int i = 0; i < BUFFER_SIZE; i++){
        promedio += buffer_ADC[i];
    }
    promedio = promedio / BUFFER_SIZE;

    //Convertimos el valor promedio de valores del ADC en voltaje real
    voltaje = (promedio * 3.3) / 4095; //4095 es el valor maximo que puede tomar el ADC

    voltaje_definido = true;


}

void TIMER2_IRQHandler(void) {

    TIM_ClearIntPending(LPC_TIM2, TIM_MR2_INT);

    if(LPC_TIM2->MR2 == t_alto_PWM){

        TIM_UpdateMatchValue(LPC_TIM2, 2, t_bajo_PWM); 
    }
    else{
        TIM_UpdateMatchValue(LPC_TIM2, 2, t_alto_PWM); 
    }

    TIM_Cmd(LPC_TIM2, ENABLE);

}

