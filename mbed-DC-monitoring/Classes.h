#include "PinNames.h"
#include "mbed.h"
#include "stm32wbxx_hal_tim.h"
#include <cstdint>

extern  TIM_HandleTypeDef htim1;
extern  TIM_HandleTypeDef htim2;
class Sensor{
    private://Variáveis Intrísecas a todos os sensores. Será o pai do sensores utilizados
        float _val;
        int _val_raw;
        int _ID;//Vai ser único e só vai ser adquirido no construtor
        int _threshold;
    public:
        Sensor(int ID){_ID = ID;}

        float getVal(){return _val;}
        void setVal(float val){_val = val;}

        int getRaw(){return _val_raw;}
        void setRaw(int val_raw){_val_raw = val_raw;}
        int getID(){return _ID;}

        int getThreshold(){return _threshold;}
        void setThreshold(int threshold){_threshold = threshold;}
        virtual void comunicacao(void) =0;
};


class Current_Sensor : public Sensor{
    private: 
        mbed::AnalogIn Corrente;

    public:
        Current_Sensor(int ID, PinName I);
        void comunicacao(void);
        void conversao(void);
};


class Velocimetro : public Sensor{
    private:
        int _val_raw_prev;
    public:
        Velocimetro(int ID);
        int getRawPrev(){return _val_raw_prev;}
        void setRawPrev(int val_raw_prev){_val_raw_prev = val_raw_prev;}

        void  comunicacao(void);
        void conversao(void);
        void update(void);
};



class Motor{
    private:
        int _ID;
        int _Cycle;// de 0 a 100
    public:
        Motor(int ID){_ID = ID;}
        int getID(){return _ID;}

        int getCycle(){return _Cycle;}
        void setCycle(int Cycle){_Cycle = Cycle;}

        void ApplyCycle(){__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, _Cycle);}//Testar se está OK
};
class Comunicator{
    private:
        bool _modo;//PC ou Celular
        bool _adminON;
        int _vals[4];//Corrente Velocimetro Aceletrometro ID_de quem ativou
        char _valsSend[4];
        int _bufferCorrente[10];
        int _bufferVeloci[10];
        int _bufferAceler[10];

        BufferedSerial serial_debug;
    public:

        Comunicator(PinName Tx, PinName Rx, int baud_rate);
        void ReceiveData();
        void SendData();

        int getModo(){return _modo;}
        void setModo(bool modo){_modo = modo;}

        bool getAdminOn(){return _adminON;}
        void setAdminOn(bool adminON){_adminON = adminON;}
        
        int getVal(int index){return _vals[index];}
        void setVal(int val,int index){_vals[index] = val;}


        int getCorrente(int index){return _bufferCorrente[index];}
        void setCorrente(int val,int index){_bufferCorrente[index] = val;}

        int getVeloci(int index){return _bufferVeloci[index];}
        void setVeloci(int val,int index){_bufferVeloci[index] = val;}


        int getAceler(int index){return _bufferAceler[index];}
        void setAceler(int val,int index){_bufferAceler[index] = val;}
        
        void UpdateBuffers();
};
