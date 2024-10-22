#include "Drivers/src/lpc17xx_timer.c"
#include "Drivers/src/lpc17xx_pinsel.c"
#include "Drivers/src/lpc17xx_dac.c"

uint32_t ultima_captura = 0;
uint32_t RE_actual = 0;
uint32_t FE_actual = 0;
uint32_t RE_anterior = 0;
uint32_t periodo = 0;
uint32_t duty [9] = {0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0};
uint8_t indice_buffer = 0;
bool ascendente = false;

void configT0(void); //Demodula la señal PWM
void configDAC(void); //Convierte la señal a un valor continuo
void configT1(void); //Controla el rate de actualizacion

int main(void){

    configT0();
    configDAC();
    configT1();

   
    while(1){ 
        //si el valor actual de captura es mayor a la ultima, significa que hubo un cambio
        if(LPC_TIM1->CR0 > ultima_captura){

            if (RE_anterior == 0){

                //Si el anterior es cero (no hubo anterior), es el primer flanco de subida
                //Al ser el primero, aun no puedo calcular nada
                RE_anterior = RE_actual;

            }
            else if(RE_anterior != RE_actual){ //si el anterior no es 0 y es distinto al actual, hubo mas de un flanco ascendente

                //Calculamos el periodo a partir de dos flancos ascendentes consecutivos
                periodo = RE_actual - RE_anterior;
                //Actualizamos el valor
                RE_anterior = RE_actual;
                
            }
            else{ //si hubo un cambio y no fue por RE, fue por FE
                uint16_t duty_aux = (FE_actual - RE_actual)/periodo * 100;  
                //Calculamos el ciclo de trabajo, a partir de cuanto tiempo estuvo en alto
                if(indice_buffer<9){ 
                    duty[indice_buffer] = duty_aux;
                    indice_buffer++; 
                }
                else{
                    //si ya se lleno el buffer, se reinicia
                    indice_buffer = 0;
                    duty[indice_buffer] = duty_aux;
                    indice_buffer++;
                }

            }

            //Calculamos un promedio entre los ultimos 10 valores que tuvo el duty
            uint32_t suma = 0;
            uint8_t divisor = 0;
            for(int i = 0; i<10; i++){
                if(duty[i] != 0){ //para no promediar con los valores que aun no se han llenado
                    divisor++;
                }
                suma += duty[i];
            }
            uint32_t promedio = suma/divisor;
        }
    
    
    } //Repetimos el bucle 
      
}

void configT0(void){ 

    //Configuramos el timer 0
    TIM_TIMERCFG_Type t1config;
    t1config.PrescaleOption = TIM_PRESCALE_TICKVAL;
    t1config.PrescaleValue = 0;

    //Configuramos modo captura
    TIM_CAPTURECFG_Type c1config;
    c1config.CaptureChanel = 0;
    //queremos detectar tanto FE como RE, e interrumpir cuando se toma una captura
    c1config.RisingEdge = ENABLE;  
    c1config.FallingEdge = ENABLE;
    c1config.IntOnCapture = ENABLE;

    //configuramos el pin por donde se toman las capturas
    PINSEL_CFG_Type pinCap;
    pinCap.Portnum = 1;
    pinCap.Pinnum = 26;
    pinCap.Funcnum = 3;
    PINSEL_ConfigPin(&pinCap);

    //Inicializamos el timer
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &t1config);
    TIM_ConfigCapture(LPC_TIM0, &c1config);
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);

}

void configDAC(void){

    //Configuramos el pin para la salida
    PINSEL_CFG_Type pinDAC;
    pinDAC.Portnum = 0;
    pinDAC.Pinnum = 26;
    pinDAC.Funcnum = 2;
    PINSEL_ConfigPin(&pinDAC);
    
    DAC_UpdateValue(LPC_DAC, 0); //hasta que no haya dutyciclo, no se envia nada
    DAC_Init(LPC_DAC);


}

void configT1(void){

    //Configuramos el timer 1
    TIM_TIMERCFG_Type t1config;
    t1config.PrescaleOption = TIM_PRESCALE_USVAL;
    t1config.PrescaleValue = 1000; //1000us son 1ms, la cuenta aumenta cada 1ms

    //Configuramos el match
    TIM_MATCHCFG_Type m1config;
    m1config.MatchChannel = 0;
    m1config.IntOnMatch = ENABLE;
    m1config.StopOnMatch = DISABLE;
    m1config.ResetOnMatch = ENABLE;
    m1config.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    m1config.MatchValue = 500; //para un rate de 0.5s necesito contar 500ms

    //Inicializamos el timer
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &t1config);
    TIM_ConfigMatch(LPC_TIM1, &m1config);
    NVIC_ClearPendingIRQ(TIMER1_IRQn);
    NVIC_EnableIRQ(TIMER1_IRQn);
    TIM_Cmd(LPC_TIM1, ENABLE);

}

void TIMER0_IRQHandler(void){
/*//ascendente empieza en false. La variable indica que edge se espera
El primer cambio de estado por el pin de capture va a ser un flanco de bajado, siguiendo la imagen. 
Entonces ante la primera interrupcion, guardamos el valor de capture en FE (falling edge) y cambiamos 
el valor de ascendente=true, indicando que el proximo que se espera es un RE (raising edge).
*/
    if(ascendente==true){ 
        RE_actual = LPC_TIM0->CR0; //si es true, estamos esperando un raising, guardamos el tiempo  
        ascendente = false;
    }
    else{
        FE_actual = LPC_TIM0->CR0; //si es false, estamos esperando un falling, guardamos el tiempo
        ascendente==true
    }

    ultima_captura = LPC_TIM0->CR0; //guardamos el valor de la ultima captura

    TIM_ClearIntCapturePending();

}


void TIMER1_IRQHandler(void){

    //Cada vez que se interrumpe, actualizamos el valor del DA
    DAC_UpdateValue(LPC_DAC, promedio);
    TIM_ClearIntCapturePending();

}