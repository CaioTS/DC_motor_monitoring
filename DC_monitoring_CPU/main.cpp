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
     vector<LogEntry> log_entries;

public:
    void addLogEntry(const  string& timestamp, const  string& event_type, const  string& intensity) {
        log_entries.push_back({timestamp, event_type, intensity});
    }

     vector<LogEntry> listEventsInDateRange(const  string& start_date, const  string& end_date) {
         vector<LogEntry> filtered_entries;
        for (const auto& entry : log_entries) {
            if (entry.timestamp >= start_date && entry.timestamp <= end_date) {
                filtered_entries.push_back(entry);
            }
        }
        return filtered_entries;
    }

     int totalActiveTimeInDateRange(const  string& start_date, const  string& end_date) {
        int total_minutes = 0;
        int maquina_estado = 0;
        int ano,mes,dia,hora,min,seg;
        int nano,nmes,ndia,nhora,nmin;
        int dano =0,dmes =0,ddia =0,dhora =0,dmin =0;
        string last_event = "S";
        sscanf(start_date.c_str(), "%d-%d-%d %d:%d",&ano,&mes, &dia, &hora, &min);

        for (const auto& entry : log_entries) {
            if (entry.timestamp >= start_date && entry.timestamp <= end_date) {
                //O mic vai mandar "I" e o horário que ligou e F e o horário que desligou,
                //Se ler um F antes de um I, é porque já estava ligado desde o início do start_date,
                //Mesma coisa serve para caso acabe com um I
                    if (entry.event_type == "I" && (last_event == "F" || last_event == "S" )) {
                        maquina_estado = 1;
                        sscanf(entry.timestamp.c_str(), "%d-%d-%d %d:%d",&ano,&mes, &dia, &hora, &min);
                        last_event = "I";
                    }
                    else if (entry.event_type == "F"&& (last_event == "I" || last_event == "S" )){
                        maquina_estado = 0;
                        sscanf(entry.timestamp.c_str(), "%d-%d-%d %d:%d",&nano,&nmes, &ndia, &nhora, &nmin);
                        dano += nano - ano;
                        dmes += nmes - mes;
                        ddia += ndia - dia;
                        dhora += nhora - hora;
                        dmin += nmin - min;
                        last_event = "F";
                    }
            }
           
        }
         if (last_event == "I"){
                sscanf(end_date.c_str(), "%d-%d-%d %d:%d",&nano,&nmes, &ndia, &nhora, &nmin);
                dano += nano - ano;
                dmes += nmes - mes;
                ddia += ndia - dia;
                dhora += nhora - hora;
                dmin += nmin - min;
            }
            total_minutes = 525960*dano +  43800*dmes + 1440*ddia + 60*dhora + dmin; 
            
        return {total_minutes};
    }
};




int main() {
    LogController logController;
    bool finish = false;
    char interaction;
    int serial_fd;
    cout << "Inicio de Interface com o Microncontrolador"<<endl;
    
    cout << "Deseja Monitorar Motor? (S) ou (N)" <<endl;
    cin >> interaction;
    
    if (interaction == 'S'){ //Inicia Monitoramento
    /*    const char* serialPort = "/dev/ttyACM0"; //Porta Serial do Microncontrolador
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

        //Baud Rate para 9600
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);
        
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
    */
    //Varrer buffer da serial pra ver se tem algo para ser lido pela serial (No caso de ter que receber a fila) e ir adicionando cada ver no vector
    vector<string> dados_sinteticos;
    dados_sinteticos.push_back("2023-01-01 12:00-F-XXX");
    dados_sinteticos.push_back("2023-01-01 18:00-I-XXX");
    dados_sinteticos.push_back("2023-01-01 18:30-E-210");
    dados_sinteticos.push_back("2023-01-01 19:00-A-098");
    dados_sinteticos.push_back("2023-01-01 19:45-F-XXX");

    char buf_time[17];//timestamp
    char buf_event[2];//Event
    char buf_intensity[4];//Intensity

    //while(read(serial_fd,buf,sizeof(buf))){
    for (const std::string& buf : dados_sinteticos) 
    {  
        buf.copy(buf_time,16,0);
        buf_time[16] = '\0';
        
        buf.copy(buf_event,1,17);
        buf_event[1] = '\0';

        buf.copy(buf_intensity,3,19);
        buf_intensity[3] = '\0';

        logController.addLogEntry(buf_time,buf_event,buf_intensity);
    }
    cout << "Letura de Filas Completa"<<endl;
    while(!finish){
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

        //Função que procura os eventos no intervalo de tempo e armazena no vetor com estrutura string,string,string
        vector<LogEntry> eventsInDateRange = logController.listEventsInDateRange(tempo_init, tempo_final); 
        cout << "Eventos no intervalo de datas:" <<  endl;
        for (const auto& entry : eventsInDateRange) {
             cout << entry.timestamp << " | " << entry.event_type << " | " << entry.intensity <<  endl;
        }
    }

    else if (interaction == 'A'){
         cin.ignore(); //Limpa cin
            string tempo_init;
            string tempo_final;
            cout << "Selecione faixa de Tempo: (ano-mes-dia hora:minuto)"<< endl;
            cout << "Tempo Inicial: "; 
            getline( cin, tempo_init); //Recebe de cin o tempo inicial da busca (TEM QUE ESTAR NO FORMATO CERTO)
            cout << "Tempo Final: "; 
            getline( cin, tempo_final);//Recebe de cin o tempo final de busca (TEM QUE ESTAR NO FORMATO CERTO)
            cout << endl;

            int total_time = logController.totalActiveTimeInDateRange(tempo_init,tempo_final);
            cout<< "Tempo total em que o acionador permanceu ativo: " << total_time; 
    }
    else if (interaction == 'P'){ // Ajustar PWM mandando valor de 0 100 pela serial
        string PWM_val;
        string PWM_send = "M";
        cout << "Duty Cycle do PWM desejado (000 a 100): ";
        cin >> PWM_val;
        PWM_send.append(PWM_val);
        /*if (write(serial_fd, &PWM_send, 4) != 4) {
            cerr << "Error writing to serial port" <<  endl;
            close(serial_fd);
            return 1;
        }
        else{
            cout << "String Transmitida: " << PWM_send << endl;
            char buf_pwm[5];
            int ret;
            cout << "PWM Atualizado com sucesso"<< endl;
            ret = read(serial_fd,buf_pwm,sizeof(buf_pwm));
            cout << "Valor Lido: " << buf_pwm << ret;
        }*/
    } 
    cout << "Fazer mais Alguma coisa? (S)im (N)ao"<<endl;
    cin >> interaction;
    if (interaction == 'N'){
        finish = true;
    }
    }
    return 0;
    }

/*
    // Character to send
    char data = 'M100';

    // Send the character
    if (write(serial_fd, &data, 4) != 4) {
         cerr << "Error writing to serial port" <<  endl;
        close(serial_fd);
        return 1;
    }

    // Close the serial port
    close(serial_fd);

     cout << "Character sent successfully!" <<  endl;

    return 0;
    */
}
