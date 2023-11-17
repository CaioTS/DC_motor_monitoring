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

void Comunicator::ReceiveData(){}

void Comunicator::SendData(){
    for (int i = 0;i<4;i++){
        _valsSend[i] = (char)_vals[i];
    }
    serial_debug.write(_valsSend,4);
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