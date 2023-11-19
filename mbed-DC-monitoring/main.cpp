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

Thread RecepcaoCPU;

uint32_t value_adc;
int angle_cnt=0;
Comunicator COM(CONSOLE_TX,CONSOLE_RX,115200);
Motor DC(3);

Queue<array<char, 3>,50> Fila;
array<char,3> Dados_Lim;
array<char,3>* Dados_Send;

int main()
{

    Current_Sensor Current(1,A0);
    Velocimetro RPM(2);
    HAL_Init();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();


    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);

    HAL_TIM_Base_Start(&htim2);

    //BufferedSerial serial_debug(CONSOLE_TX, CONSOLE_RX,115200);

    //SerialStream<BufferedSerial> debug_Stream(serial_debug);
    //debug_Stream.printf("Hello World From MBED- OS\n");


    DC.setCycle(100);
    DC.ApplyCycle();


    while (true) {
        ThisThread::sleep_for(100ms);


        COM.setVal((int)Current.getVal(),0);
        COM.setVal((int)RPM.getVal(),1);
        COM.setVal((int)35,2);
        COM.setVal((int)32,3);
    
        //Leitura e Conversão dos dados
        Current.comunicacao();
        RPM.comunicacao();
        


        if (COM.getAdminOn()){//Verifica se ADMIN está conectado

            //Se tem dados na fila, transmitir.
            do {
                Fila.try_get(&Dados_Send);
                COM.TransmitArray(*Dados_Send);
            }while(!Fila.empty());

            //Depois Transmite Continuamente
            
            COM.setVal((int)(Current.getVal()*100),0);//Valor de corrente multiplicado para não perder precisão
            COM.setVal((int)RPM.getVal(),1);//Valor de RPM referente a velocidade
            COM.setVal((int)DC.getCycle(),2);//Ciclo do PWM atual sendo utilizado
            COM.setVal((int)67,3);//Valor de Segurança para garantir que os dados estão sendo lidos corretamente

            COM.SendVals();
        }

        else { // Modo offline onde ele manda os dados para a fila caso o detecte alguma anomalia
            if (Current.getVal()>1.5){ // Anomalia: Corrente maior que 1.5 A
                Dados_Lim[0] = (char)Current.getID();
                Dados_Lim[1] = (char)(Current.getVal()*100);
                Dados_Lim[2] = (char)(RPM.getVal());
                Dados_Lim[3] = (char)DC.getCycle();
                Fila.try_put(&Dados_Lim);
            }
            if (RPM.getVal() < 10 and Current.getVal() > 0.2){// Anomalia: RPM muito baixo e corrente em nível OK, Sugere muito peso no motor
                Dados_Lim[0] = (char)RPM.getID();
                Dados_Lim[1] = (char)(Current.getVal()*100);
                Dados_Lim[2] = (char)(RPM.getVal());
                Dados_Lim[3] = (char)DC.getCycle();
                Fila.try_put(&Dados_Lim);
            }

        }

        COM.ReceiveData(); //Verifica se Recebeu algum dado
            if (COM.getRvalid() and (COM.getReceived()== 5)){ //Se dado for válido, e foi refente ao PWM, altera o valor do PWM
                    DC.setCycle(COM.getPWM());
                    DC.ApplyCycle(); //Aplica o valor do PWM

                } 
        
        
    }
}
