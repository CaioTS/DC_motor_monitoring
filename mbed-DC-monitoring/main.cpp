#include "mbed.h"
#include "Main.h"
#include "stm32wbxx_hal_rcc.h"
#include "Classes.h"




#include "AnalogIn.h"
#include "PinNames.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

//AnalogIn   Corrente(A0);

Thread RecepcaoCPU;

uint32_t value_adc;
int angle_cnt=0;
Comunicator COM(CONSOLE_TX,CONSOLE_RX,115200);
Motor DC(3);

int main()
{

    Current_Sensor Current(1,A0);
    Velocimetro RPM(2);
    HAL_Init();
    MX_GPIO_Init();
    //MX_DMA_Init();
    //MX_ADC1_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();

    //HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&value_adc,1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);

    HAL_TIM_Base_Start(&htim2);

    //BufferedSerial serial_debug(CONSOLE_TX, CONSOLE_RX,115200);

    //SerialStream<BufferedSerial> debug_Stream(serial_debug);
    //debug_Stream.printf("Hello World From MBED- OS\n");

    RecepcaoCPU.start(Recepcao_CPU);

    DC.setCycle(100);
    DC.ApplyCycle();

    while (true) {
        ThisThread::sleep_for(100ms);
        Current.comunicacao();
        RPM.comunicacao();
        

       /* if (DC.getCycle()<100){
            DC.setCycle(DC.getCycle()+1);
        }
        else DC.setCycle(0);
        */
        DC.ApplyCycle();
        COM.setVal((int)Current.getVal(),0);
        COM.setVal((int)RPM.getVal(),1);
        COM.setVal((int)35,2);
        COM.setVal((int)32,3);

        //COM.SendData("Vals");

        //debug_Stream.printf("Corrente: %d , %d   RPM: %d , %d  %d    PWM : %d\n",(int)Current.getVal(),Current.getRaw(),(int)(RPM.getVal()*100),RPM.getRaw(),RPM.getRawPrev(),DC.getCycle());
    
        //Ler dados

        //Converter Dados

        //Verifica se ADMIN está conectado
        COM.ReceiveData();
            if (COM.getRvalid() and (COM.getReceived()== 5)){ //Dado válido e alterar PWM
                    COM.SendData("XX");
                    DC.setCycle(COM.getPWM());
                } 
        DC.ApplyCycle();
        //Se sim - transmite tudo continuamente
            //Fila tem dados?
            //Se sim, transmite pela CPU ou Bluetooth dependendo do modo
        //Se não armazena no buffer 
            //Se passar algum thrshold manda o buffer e o ID do sensor para fila. e Depois manda para fila todos os dados até a emergência acabar

        


    
    
    
    }
}

//Criar uma thread só pra recepção de dados da serial
    void Recepcao_CPU(){
        while(1){
            COM.SendData("Thread");
        }  
        }