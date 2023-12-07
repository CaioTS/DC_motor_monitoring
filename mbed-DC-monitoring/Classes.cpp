#include "Main.h"
#include "mbed.h"
#include "Classes.h"
#include "AnalogIn.h"
#include "PinNames.h"

//Construtor Classe do Ampeŕimetro com ID e pinName
Current_Sensor::Current_Sensor(int ID, PinName I) : Corrente(I) , Sensor(ID){}

void Current_Sensor :: comunicacao(){
    setRaw(Corrente.read_u16());
    conversao();//Converte para valores que fazem sentido
}

void Current_Sensor :: conversao(){
    setVal(getRaw()*3.3/(65536));//Tensão/Resitencia e máximo nível para se obter no pino analógico
}


Velocimetro::Velocimetro(int ID):Sensor(ID){}

void Velocimetro::comunicacao(){
    setRaw(__HAL_TIM_GetCounter(&htim2));
    conversao();//Converte para valores que fazem sentido
    update();//Atualiza valor de referência para calculo de velocidade
}

void Velocimetro::conversao(){
    int delta = getRaw() - getRawPrev();
    delta = (delta<0) ? delta + 1000 : delta; //Lida com contador do TIMER
    setVal(delta/10000.0*16);//Converte para rpm
}

void Velocimetro::update(){//Ataliza valor de Referência
    setRawPrev(getRaw());
}

//Construtor da classe de Comunicação com pino tx e rx , e bauad rate da serial
Comunicator::Comunicator(PinName Tx, PinName Rx, int baud_rate):serial_debug(Tx,Rx,baud_rate){}

void Comunicator::ReceiveData(){
    char buf[4] = {0};
    char aux[1] = {0};
    if (char num = serial_debug.read(buf, sizeof(buf))){
        setReceived(0);//Recebeu dados novos
        setRvalid(true);
        //serial_debug.write(buf,sizeof(buf));
        //SendData(buf);
        switch (buf[0]){
            case 'A': //Ativa modo Admin 41
                setAdminOn(true);
                maquina_estado =0;
            break;
            case 'D':// Desativa modo Admin 44
                //SendData("Admin Desconectado");
                setAdminOn(false);
            break;
            case 'P'://modo para PC 50
                //SendData("Modo PC");
                setModo(false);
            break;
            case 'C'://Modo para Celular 43
                //SendData("Modo Celular");
                setModo(true);
            break;
            case 'M':// Altera PWM 4D
                //SendData("Alterando PWM");
                setReceived(5);
                setPWM((int)(buf[1] - '0')*100 + (int)(buf[2] - '0')*10 + (int)(buf[3] - '0'));
                if (getPWM() > 100 or getPWM() < 0){setRvalid(false);}
                SendData(buf);
                break;
            case 'S'://53 - Mande o Dado
                maquina_estado = 2;
                break;
            case 'B'://42   Dado Bom
                maquina_estado = 0;
                break;
            case 'W'://57 - Dado foi mandado errado , remandar
                maquina_estado = 3;
                break;
            default:
            //setRvalid(false);//Dado Inválido
            break;
        }

    }
}

void Comunicator::SendData(char * val){ //Manda strings pra serial, foi utililziada para debug
     
    serial_debug.write(val,strlen(val));
}

void Comunicator:: TransmitArray(vector <char> & val) { //Trasmite evento pela seriak
    char* charArray = val.data();//Converte vector <char> em char *
    if(!getModo())serial_debug.write(charArray,22); //Manda os 22 bytes do evento
    
}

