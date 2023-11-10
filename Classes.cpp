class Sensor{
    private://Variáveis Intrísecas a todos os sensores. Será o pai do sensores utilizados
        int _val;
        int _ID;//Vai ser único e só vai ser adquirido no construtor
    public:
        Sensor(int ID){_ID = ID;}

        int getVal(){return _val;}
        void setVal(int val){_val = val;}

        int getID(){return _ID;}
};






class Comunicator{
    private:
        int _vals[3];
};
