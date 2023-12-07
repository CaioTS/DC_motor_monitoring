#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <termios.h>

using namespace std;


struct LogEntry {
     string timestamp; //Tempo que ocorreu
     string event_type; // Tipo de Evento: Se foi Corrente ou RPM, ou inicialização do mic.
     string intensity;//Valor Do Evento
};

class LogController {
private:
     vector<LogEntry> log_entries; //Vetor composto pela estrutura de strings que representa os dados seriais transmitidos pela CPU

public:
    void addLogEntry(const  string& timestamp, const  string& event_type, const  string& intensity) { //Adiciona dado da fila ao vector log_entries
        log_entries.push_back({timestamp, event_type, intensity});
    }
//Lista os eventos no intervalo de tempo desejado
vector<LogEntry> listEventsInDateRange(const  string& start_date, const  string& end_date) { 
    vector<LogEntry> filtered_entries;
    for (const auto& entry : log_entries) { 
        //laço for que acessa todos os log_entries do vetor de cada vez 
        //Limita a apenas os eventos dentro da faixa de tempo observando os time-stamps
        if (entry.timestamp >= start_date && entry.timestamp <= end_date) { 
            filtered_entries.push_back(entry);
            //Adiciona dados da log_entries ao filtered_entries que estão dentro da faixa de tempo
        }
   }
return filtered_entries; //Retorna Vector com os eventos do intervalo de tempo
}
     //Função que verifica quanto tempo o controlador ficou ligado
     int totalActiveTimeInDateRange(const  string& start_date, const  string& end_date) { 
        int total_minutes = 0;
        int maquina_estado = 0;
        int ano,mes,dia,hora,min,seg;
        int nano,nmes,ndia,nhora,nmin;
        int dano =0,dmes =0,ddia =0,dhora =0,dmin =0;
        string last_event = "S"; //Inicia com ultimo Evento em S de start, para caso no inicio da faixa de tempo o controlador já estivesse ligado
        sscanf(start_date.c_str(), "%d-%d-%d %d:%d",&ano,&mes, &dia, &hora, &min); //Converte a string dos dados em variáveis inteiras

        for (const auto& entry : log_entries) { //Laço for que passa por todos os eventos da log_entries
            if (entry.timestamp >= start_date && entry.timestamp <= end_date) { //Limita para os eventos dentro da faixa de tempo
                //O mic vai mandar "I" e o horário que ativou o controlador e F e o horário que desligou,
                //Se ler um F antes de um I, é porque já estava ligado desde o início do start_date,
                //Mesma coisa serve para caso acabe com um I
                    if (entry.event_type == "I" && (last_event == "F" || last_event == "S" )) {
                        // Se foi lido como uma ativação (I) e o ultimo evento foi desativação ou é o primeiro evento de atuação
                        sscanf(entry.timestamp.c_str(), "%d-%d-%d %d:%d",&ano,&mes, &dia, &hora, &min);//Nova referencai para o horário de ativação
                        last_event = "I"; //Indica que o ultimo evento foi uma atuação
                    }
                    else if (entry.event_type == "F"&& (last_event == "I" || last_event == "S" )){
                        // Se foi lido como uma desativação (I) e o ultimo evento foi ativação ou é o primeiro evento de atuação
                        sscanf(entry.timestamp.c_str(), "%d-%d-%d %d:%d",&nano,&nmes, &ndia, &nhora, &nmin);//Novos dados que indicam fim da atuação
                        //Obtem a variação do tempo da ativação até a desativação
                        dano += nano - ano; 
                        dmes += nmes - mes;
                        ddia += ndia - dia;
                        dhora += nhora - hora;
                        dmin += nmin - min;
                        last_event = "F";//Indica que o ultimo evento foi uma desativação
                    }
            }
        }
         if (last_event == "I"){ 
            // Se percorreu todo o intervalo de tempo e o ultimo evento foi uma atuação, 
            //devemos considerar que até o fim do intervalo de tempo analisado, o controlador permaneceu ativado
                sscanf(end_date.c_str(), "%d-%d-%d %d:%d",&nano,&nmes, &ndia, &nhora, &nmin);//Adquire dados de fim do intervalo como sendo fim da atuação
                dano += nano - ano;
                dmes += nmes - mes;
                ddia += ndia - dia;
                dhora += nhora - hora;
                dmin += nmin - min;
            }
            total_minutes = 525960*dano +  43800*dmes + 1440*ddia + 60*dhora + dmin;  //Soma todas as variações de tempo convertidas para minutos
            
        return {total_minutes};
    }
};

    LogController logController; //Classe que armazena FILA e faz as pesquisas nos intervalos de tempo
    int serial_fd; //Interface Serial
    bool finish = false; // Se o admin deseja encerrar o monitoramento

void Recebe_Dado(){
        //Adiciona todos os dados no vector do LogController
    
    std::cout << "Iniciou Thread";
    char buf_time[17];//timestamp
    char buf_event[2];//Event
    char buf_intensity[4];//Intensity
    char serial[1] = {0};//Armzanena cada byte enviado
    char buf[23] = {0};//Buf que será armazenado nos Logs
    int bytesRead;
    char aux[1] = {'S'};//Autoriza mic a mandar o evento
    char aux2[1] = {'B'};//Evento recebido corretamente
    char aux3[1] = {'W'};//Evento recebido incorretamente
    int maquina_estado = 0;
    while(!finish){
        
        if (maquina_estado == 0){//Espera receber 'O'
            read(serial_fd,&serial,1);
            if (serial[0] == 'O'){maquina_estado = 1;write(serial_fd,aux,1);}
            else maquina_estado = 0;
        }
        else if (maquina_estado == 1){//Le 22 bytes para montar string
        for (int i = 0; i<22;i++){
        bytesRead = read(serial_fd, &serial, 1);
        buf[i] = serial[0];
        }
        buf[22] = '\0';
        // Se buf[10] não for um espaço, evento foi enviado incorretamente
        if (buf[10] == ' '){write(serial_fd,aux2,1);maquina_estado = 2;}//Dado ok
        else {write(serial_fd,aux3,1);maquina_estado = 1;}//Dado não Ok
        }

        else if (maquina_estado == 2){ //Converte buf nas 3 strings para o log
            //os 16 primeiros bytes são sobre o tempo
            for (int i = 0; i<16;i++){buf_time[i] = buf[i];}
            buf_time[16] = '\0';

            //O byte 18 é o tipo de evento
            buf_event[0] = buf[17];
            buf_event[1] = '\0';

            //do byte 20 a 22 se refere a intesidade do evento
            for (int i = 0; i<3;i++){buf_intensity[i] = buf[i+19];}
            buf_intensity[3] = '\0';
            maquina_estado = 0;
            
            //Adiciona dados de log
            logController.addLogEntry(buf_time,buf_event,buf_intensity);
            std:: cout << buf<<endl;

        }
    }
}


int main() {
    char interaction;//Armazena variável dos cins
    cout << "Inicio de Interface com o Microncontrolador"<<endl;
    
    cout << "Deseja Monitorar Motor? (S) ou (N)" <<endl;
    cin >> interaction;
    
    if (interaction == 'S'){ //Inicia Monitoramento

        const char* serialPort = "/dev/ttyACM1"; //Porta Serial do Microncontrolador
        cout << "Estabelecendo Comunicacao Serial" << endl;
        serial_fd = open(serialPort, O_RDWR); //Abre para read e write a porta serial
        if (serial_fd == -1) {
             cerr << "Error opening serial port " << serialPort <<  endl;
            return 1;
        

    }
    // Modifica Configuração da Serial
        struct termios tty;
        if (tcgetattr(serial_fd, &tty) != 0) {
            std::cerr << "Error getting serial port settings" << std::endl;
            return 1;
        }

        //Baud Rate para 115200
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);
        
        // Bits de Daddos (8 bits)
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;

        // Stop Bits (1 stop bit)
        tty.c_cflag &= ~CSTOPB;

        // Paridade (sem paridade)
        tty.c_cflag &= ~PARENB;
        // Aplica as novas configurações
        if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
            std::cerr << "Error setting serial port attributes" << std::endl;
            return 1;
        }
    
    vector<string> dados_sinteticos; //Criação dos dados sintéticos para teste
    dados_sinteticos.push_back("2023-01-01 12:00-F-XXX");
    dados_sinteticos.push_back("2023-01-01 18:00-I-XXX");
    dados_sinteticos.push_back("2023-01-01 18:30-E-210");
    dados_sinteticos.push_back("2023-01-01 19:00-A-098");
    dados_sinteticos.push_back("2023-01-01 19:45-F-XXX");


    write(serial_fd,"AXXX",5);
        
    thread Recebe_log(Recebe_Dado);

    cout << "Letura de Filas Ativada"<<endl;
    while(!finish){ //Enquanto o admin não cancela o monitoramento, ele repete a interface com o admin.
    cout << "Proxima Execucao: Verificar Logs(L), Ajustar PWM(P) ou Verificar Tempo Ativado(A)"<<endl;
    cin >> interaction;

    if (interaction == 'L'){ //Ler Logs da faixa de tempo
        cin.ignore(); //Limpa cin
        string tempo_init;
        string tempo_final;
        cout << "Selecione faixa de Tempo: (ano-mes-dia hora:minuto)"<< endl;
        cout << "Tempo Inicial: "; 
        getline( cin, tempo_init); //Recebe de cin o tempo inicial da busca (TEM QUE ESTAR NO FORMATO CERTO)
        cout << "Tempo Final: "; 
        getline( cin, tempo_final);//Recebe de cin o tempo final de busca (TEM QUE ESTAR NO FORMATO CERTO)
        cout << endl;

        //Função que procura os eventos no intervalo de tempo e armazena no vector com estrutura string,string,string
        vector<LogEntry> eventsInDateRange = logController.listEventsInDateRange(tempo_init, tempo_final); 
        cout << "Eventos no intervalo de datas:" <<  endl;
        for (const auto& entry : eventsInDateRange) { //Printa eventos dentro da faixa de tempo selecionada
             cout << entry.timestamp << " | " << entry.event_type << " | " << entry.intensity <<  endl;
        }
    }

    else if (interaction == 'A'){//Calcular tempo ativo do controlador
         cin.ignore(); //Limpa cin
            string tempo_init;
            string tempo_final;
            cout << "Selecione faixa de Tempo: (ano-mes-dia hora:minuto)"<< endl;
            cout << "Tempo Inicial: "; 
            getline( cin, tempo_init); //Recebe de cin o tempo inicial da busca (TEM QUE ESTAR NO FORMATO CERTO)
            cout << "Tempo Final: "; 
            getline( cin, tempo_final);//Recebe de cin o tempo final de busca (TEM QUE ESTAR NO FORMATO CERTO)
            cout << endl;

            int total_time = logController.totalActiveTimeInDateRange(tempo_init,tempo_final);//Função que calcula tempo de ativação dentro da faixa de tempo
            cout<< "Tempo total em que o acionador permanceu ativo: " << total_time << " minutos" << endl; 
    }
    else if (interaction == 'P'){ // Ajustar PWM mandando valor de 0 100 pela serial
        string PWM_val;
        string PWM_send = "M";//Valor para que o micocontrolador indica 
        char buf[4] = {0};
        //que é para alterar o microncontrolador
        cout << "Duty Cycle do PWM desejado (000 a 100): ";
        cin >> PWM_val;
        PWM_send.append(PWM_val);
        const char* charBuffer = PWM_send.c_str();

        if (write(serial_fd, charBuffer, 4) != 4) {
            cerr << "Error writing to serial port" <<  endl;
            close(serial_fd);
            return 1;
        }
        else{
            cout << "String Transmitida: " << PWM_send << endl;
            char buf_pwm[15];
            int ret;
            cout << "PWM Atualizado com sucesso"<< endl;
            this_thread::sleep_for(500ms);
        }
    } 
    cout << "Fazer mais Alguma coisa? (S)im (N)ao"<<endl;
    cin >> interaction;
    if (interaction == 'N'){
        finish = true;//Encerra Programa
        write(serial_fd,"DXXX",5);
        Recebe_log.join();

    }
    }
}
    return 0;
}