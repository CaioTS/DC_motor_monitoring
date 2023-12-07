#include "DigitalOut.h"

#include "ThisThread.h"
#include "Thread.h"
#include "cmsis_os2.h"
#include "mbed.h"
#include "Main.h"
#include "stm32wbxx_hal_rcc.h"
#include "Classes.h"


#include "AnalogIn.h"
#include "PinNames.h"

#include "ClockCalendar.h"
#include <string>
#include <vector>

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;


uint32_t value_adc;
int angle_cnt=0;
int maquina_estado = 0;

Comunicator COM(CONSOLE_TX,CONSOLE_RX,115200); //Inicializa serial dentro da classe COM
Motor DC(3);//Motor e ID 3

Queue<vector <char>,10> Fila; //Fila de 10 vetores de char
vector <char>* Dados_Send; // Variável que interage com FILA

 
Thread Clock_Thread(osPriorityNormal, 100, nullptr, nullptr); //Thread que conta os segundos
Thread RecepcaoCPU(osPriorityNormal,500,nullptr,nullptr); // Threas que cuida da Recepção Serial com a COU

ClockCalendar Relogio(12,6,2023,9,40,10,1);//ClockCalenadr desenvolvido em aula inicializando com a data 
Current_Sensor Current(1,A0);//Sensor de Corrente no pino A0 ID 1
Velocimetro RPM(2);//Velocimetro com ID 2

DigitalOut led1(LED1);//Led que indica os segundos
DigitalOut led2(LED2);//Led que indica se recebeu dado
DigitalOut led3(LED3);//Led que indica de aconteceu algum evento

void  Real_Time_Update() //Função da Thread do relógio
{
    while(1){
        ThisThread::sleep_for(1s); //A cada 1 segundo avança no tempo
        Relogio.advance();
        led1 = !led1;

    }

}

void RecebeuCPU()//Thread da recepção Serial
{
    while(1){
        ThisThread::sleep_for(30ms);//Para não travar o recebimento dos dados
        led2 = 1;
        COM.ReceiveData(); //Verifica se Recebeu algum dado
        led2 = 0;
    }
}


vector <char> InttoStr(int x,int strings_n) //Função que converte int em string 
{

    std::vector<char> digits;
    while (x > 0) {
        int digit = x % 10;  // Pega ultimo Digito
        char charDigit = '0' + digit;  // Converte pra char
        digits.push_back(charDigit); // Coloca char na parte de trás do vetor digits

    
        x /= 10; //Divide para que se tenha o próximo digito
    }
    digits.push_back('0'); //Adiciona 0s por causa do protocolo
    digits.push_back('0');
    digits.push_back('0');
 
    std::reverse(digits.begin(), digits.end());   // INverte para que estejam na ordem correta
    std::vector<char> selectedInterval(digits.end() - strings_n, digits.end()); //Seleciona apenas numeros de digitos especificados por n_string

    return selectedInterval;
}


vector <char> VectorSend(){ //Gera vetor char com as informações de tempo para enviar a CPU
        int year,month,day,hour,minute,second,pm;
        vector <char> string_format;
        Relogio.readCalendar(month, day, year);//Le mes dia e ano
        Relogio.readClock(hour, minute,second, pm);// Le hora, minuto, segundo

        vector <char> temp;
        temp = InttoStr(year,4);//Converte ano em string com 4 digitoos
        string_format.insert(string_format.end(), temp.begin(), temp.end());//Coloca dado no vetor total
        string_format.push_back('-');
        temp = InttoStr(month,2);//Converte meses em string com 2 digitos
        string_format.insert(string_format.end(), temp.begin(), temp.end());
        string_format.push_back('-');
        temp = InttoStr(day,2);//Converte dia em string de 2 digitos
        string_format.insert(string_format.end(), temp.begin(), temp.end());
        string_format.push_back(' ');

        temp = InttoStr(hour + 12*pm,2); //Converte hora em string com dois digitos
        string_format.insert(string_format.end(), temp.begin(), temp.end());
        string_format.push_back(':');
        temp = InttoStr(minute,2);//Converte minutos em string com dois dígitos
        string_format.insert(string_format.end(), temp.begin(), temp.end());
        string_format.push_back('-');
        return string_format;//Retorna vetor de char com todas as informaçãoes de tempo

}

vector<char> aux_Fila;//Conta númeoro de eventos na fila

int main()
{
    //Inicia Threads
    Clock_Thread.start(Real_Time_Update);
    RecepcaoCPU.start(RecebeuCPU);

    //Inicia periféricos e Timers
    HAL_Init();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();

    //Inicia clockd dos pwm
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);

    HAL_TIM_Base_Start(&htim2);

    //Começa ciclo do pwm no máximo
    DC.setCycle(30);
    DC.ApplyCycle();

    vector <char> string_format;//string que é armazenada na lista
    vector <char> temp;
    //Auxiliares para que eventos repetidos não aconteçam
    bool aux_C = true,aux_V = true, aux_init = true;


    COM.setAdminOn(false);//Começa sem admin conectado
    maquina_estado = 0;
    led3 = 0;
    ThisThread::sleep_for(1s);//Dorme para um segundo para garantir inicizalização correta de tudo

    while (true) {

        //Leitura e Conversão dos dados
        Current.comunicacao();
        RPM.comunicacao();
        
        if (COM.getAdminOn()){//Verifica se ADMIN está conectado
            //Se tem dados na fila, transmitir.
            
            while(!Fila.empty()){
                aux_Fila = InttoStr(Fila.count(),2);
                if (maquina_estado == 0 ){COM.SendData("O");maquina_estado = 0;}//Dado disponível para envio
                else if (maquina_estado == 1){}//Espra ler o S ;
                else if (maquina_estado == 2){//Armazena dado a ser transmitido
                    Fila.try_get(&Dados_Send);
                    maquina_estado =3;
                }

                else if (maquina_estado == 3){//Transmite evento
                    COM.TransmitArray(*Dados_Send);
                    maquina_estado = 4;
                }
                else if (maquina_estado == 4){}//Espera Status do Dado

                ThisThread::sleep_for(200ms);

            }

            if ((COM.getReceived()== 5)){ //Se dado for válido, e foi refente ao PWM, altera o valor do PWM
                    COM.setReceived(0);
                    DC.setCycle(COM.getPWM());
                    DC.ApplyCycle(); //Aplica o valor do PWM
                    led3 = 1;

                } 

        }

            if (aux_init){
                aux_init =false;
                //Armazena evento de inicialização do microcontrolador
                string_format = VectorSend();//Adquire informações do tempo
                string_format.push_back(('I'));//Tipo de evento Inicialização
                string_format.push_back(('-'));
                temp =  InttoStr((int)(Current.getVal()*100),3);//Pega valor aletário
                string_format.insert(string_format.end(), temp.begin(), temp.end());
                string_format.push_back('\0');//Adiciona Fim de string 
                Fila.try_put(&string_format);//Adciona a lista evento de inicialização (BUG) não le a primeira tranmissão, por isso envia essa duas vezes
                Fila.try_put(&string_format);//Adciona a lista evento de inicialização

                string_format.clear();//limpa vetor de char

            }
            if (Current.getVal()>1.5 and aux_C){ // Anomalia: Corrente maior que 1.5 A 
                aux_C = false;//Só remanda se baixar deste nível de corrente
                string_format = VectorSend();//Adquire informações do tempo
                string_format.push_back(('A'));//Tipo de evento Corrente elevada
                string_format.push_back(('-'));
                temp =  InttoStr((int)(Current.getVal()*100),3);//Valor de corrente multiplicada por 100 para converter x,yz para xyz
                string_format.insert(string_format.end(), temp.begin(), temp.end());
                string_format.push_back('\0');
                Fila.try_put(&string_format);
                led3 = 1; //Acende led para avisar evento
                string_format.clear();//limpa vetor de char
                ThisThread::sleep_for(100ms);
            }
            else if (Current.getVal() <=1.5){aux_C = true; led3 = 0;ThisThread::sleep_for(100ms);}//Voltou a normalidade, fica habil a detectar eventos
            if (RPM.getVal() < 10 and Current.getVal() > 0.2 and aux_V ){// Anomalia: RPM muito baixo e corrente em nível OK, Sugere muito peso no motor
                aux_V = false;//Só remanda se voltar a normalidade novamente
                string_format = VectorSend();//Adquire informações do tempo
                string_format.push_back(('V'));//Tipo de evento velocimetro
                string_format.push_back(('-'));
                temp =  InttoStr((int)(RPM.getVal()*100),3);//Pega valor de RPM
                string_format.insert(string_format.end(), temp.begin(), temp.end());
                Fila.try_put(&string_format);
                led3 =1;
                Fila.try_get(&Dados_Send);

                string_format.clear();


            }
            else if (RPM.getVal()> 10){aux_V = true;ThisThread::sleep_for(100ms);}//Voltou a normalidade, fica habil a detectar eventos
        
        
    }
}
