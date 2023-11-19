#include "Main.h"
#include "mbed.h"
#include "Classes.h"

#include "AnalogIn.h"
#include "PinNames.h"

Current_Sensor::Current_Sensor(int ID, PinName I) : Corrente(I) , Sensor(ID){}

void Current_Sensor :: comunicacao(){
    setRaw(Corrente.read_u16());
    conversao();
}

void Current_Sensor :: conversao(){
    setVal(getRaw()*33.3/(65536));//Resistor Shunt de 0.1 Ohms
}


Velocimetro::Velocimetro(int ID):Sensor(ID){}

void Velocimetro::comunicacao(){
    setRaw(__HAL_TIM_GetCounter(&htim2));
    conversao();
    update();
}

void Velocimetro::conversao(){
    int delta = getRaw() - getRawPrev();
    delta = (delta<0) ? delta + 1000 : delta;
    setVal(delta/10000.0*16);
}

void Velocimetro::update(){
    setRawPrev(getRaw());
}


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
                SendData("Admin Conectado");
                setAdminOn(true);
            break;
            case 'D':// Desativa modo Admin 44
                SendData("Admin Desconectado");
                setAdminOn(false);
            break;
            case 'P'://modo para PC 50
                SendData("Modo PC");
                setModo(false);
            break;
            case 'C'://Modo para Celular 43
                SendData("Modo Celular");
                setModo(true);
            break;
            case 'M':// Altera PWM 4D
                SendData("Alterando PWM");
                setReceived(5);
                setPWM((int)(buf[1] - '0')*100 + (int)(buf[2] - '0')*10 + (int)(buf[3] - '0'));
                if (getPWM() > 100 or getPWM() < 0){setRvalid(false);}
            default:
                setRvalid(false);//Dado Inválido
            break;
        }

    }
}

void Comunicator::SendData(char * val){
     
    serial_debug.write(val,sizeof(val));
}

void Comunicator::SendVals(){

    for (int i = 0;i<4;i++){
        _valsSend[i] = (char)_vals[i];
    }

    if (!getModo())serial_debug.write(_valsSend,sizeof(_valsSend));
}


void Comunicator:: TransmitArray(std::array<char, 3> vetor) {
    char aux[4] = {0};
    for (int i =0 ; i<4 ;i++){
        aux[i] = vetor[i];
    }
    if(!getModo())serial_debug.write(aux,sizeof(aux));
}
void Comunicator::UpdateBuffers(){
    if (!getAdminOn()){
    for (int i = 1;i<10;i++){
        setCorrente(getCorrente(i-1), i);
        setAceler(getAceler(i-1), i);
        setVeloci(getVeloci(i-1), i);
    }
    setCorrente(getVal(0), 0);
    setVeloci(getVal(1), 0);
    setAceler(getVal(2), 0);
    }
}
