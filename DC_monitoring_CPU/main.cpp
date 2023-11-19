#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
int main() {
    cout << "Inicio de Interface com o Microncontrolador"<<endl;
    
    const char* serialPort = "/dev/ttyACM0";

    int serial_fd = open(serialPort, O_RDWR);
    if (serial_fd == -1) {
        std::cerr << "Error opening serial port " << serialPort << std::endl;
        return 1;
    }

    // Character to send
    char data = 'M100';

    // Send the character
    if (write(serial_fd, &data, 4) != 4) {
        std::cerr << "Error writing to serial port" << std::endl;
        close(serial_fd);
        return 1;
    }

    // Close the serial port
    close(serial_fd);

    std::cout << "Character sent successfully!" << std::endl;

    return 0;
}
