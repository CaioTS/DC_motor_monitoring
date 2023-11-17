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

/* USER CODE BEGIN PV */
uint32_t value_adc;
int angle_cnt=0;

int main()
{

    Current_Sensor Current(1,A0);
    Velocimetro RPM(2);
    Comunicator COM(CONSOLE_TX,CONSOLE_RX,115200);

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
    //BufferedSerial serial_CPU(PC_12,PD_2,4800);

    //SerialStream<BufferedSerial> debug_Stream(serial_debug);
    //debug_Stream.printf("Hello World From MBED- OS\n");

    while (true) {
        ThisThread::sleep_for(500ms);
        Current.comunicacao();
        RPM.comunicacao();

        COM.setVal((int)Current.getVal(),0);
        COM.setVal((int)RPM.getVal(),1);
        COM.setVal((int)35,2);
        COM.setVal((int)32,3);

        COM.SendData();
        //debug_Stream.printf("Corrente: %d , %d   RPM: %d , %d  %d\n",(int)Current.getVal(),Current.getRaw(),(int)(RPM.getVal()*100),RPM.getRaw(),RPM.getRawPrev());
    }
}

