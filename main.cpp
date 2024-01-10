#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "measuremodule.h"
#include "sensormeasure.h"

using namespace std;

#define PORT 12778

MeasureModule* mm;

string formatError(string error) {
    return "{\"success\": false, \"error\": \"" + error + "\"}\0";
}
string formatSuccess() {
    return "{\"success\": true}\0";
}
string formatSendData(SensorMeasure* data) {
    return "{\"success\": true, \"data\": {\"CO2\": " + data->getCO2() + ", \"temperature\": " + data->getTemperature() + ", \"humidity\": " + data->getHumidity() + ", \"pressure\": " + data->getPressure() + ", \"O2\": " + data->getO2() + ", \"luminosity\": " + data->getLuminosity() + "}}\0";
}

vector<string> getArgs(char* buffer) {
    char separator = ' ';
    string str(buffer);
    str.erase(str.find_last_not_of(" \n\r\t")+1);
    vector<string> args;

    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {

        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
            string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            args.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

    return args;
}

// Syntax : RESET
string resetSensors() {
    mm->reset();
    return formatSuccess();
}

// Syntax : SET_ALTITUDE <ALTITUDE>
string setAltitude(string alt) {
    int altitude = 0;
    try {
        altitude = stoi(alt);
    } catch (...) {
        return formatError("L'argument de l'altitude est invalide.");
    }

    mm->setAltitude(altitude);

    return formatSuccess();
}

// Syntax : GET_ERRORS
string getErrors() {
    return mm->get_errors();
}

// Syntax : GET_MEASURE
string getSensorMeasure() {
    SensorMeasure* data = mm->get();
    if (data == nullptr) {
        if (mm->isInitialising()) {
            return formatError("Le dispositif de mesure n'a fini de s'initialiser.");
        } else {
            return formatError("Le dispositif de mesure a probablement été intérrompu à la suite d'une erreur. Pour plus d'information, consultez les erreurs avec GET_ERRORS puis tentez de le réinitialiser avec RESET.");
        }
    }

    string res = formatSendData(data);
    cout << "--------------------" << endl;
    cout << " CO2: " << data->getCO2() << "%/vol" << endl;
    cout << " TEMP: " << data->getTemperature() << "°C" << endl;
    cout << " HUMID: " << data->getHumidity() << "%" << endl;
    cout << " PRESS: " << data->getPressure() << "Pa" << endl;
    cout << " O2: " << data->getO2() << endl;
    cout << " LUM: " << data->getLuminosity() << "" << endl;

    delete data;
    return res;
}

// Handle client requests
void handleClient(int clientSocket) {
    while (true) {
        char buffer[256];
        ssize_t bytesRead;

        // Read data from the client
        bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            perror("Error reading from socket");
            close(clientSocket);
            return;
        } else if (bytesRead == 0) {
            // Connection closed by the client or lost
            cout << "Client disconnected (connection lost)" << endl;
            close(clientSocket);
            return;
        }

        // Null-terminate the received data
        buffer[bytesRead] = '\0';

        // Call the appropriate function based on the received message
        string response = "";
        vector<string> args = getArgs(buffer);

        if (args.size() == 0) {
            response = formatError("No command provided");
        } else {
            string cmd = args[0];

            if (cmd == "RESET") {
                response = resetSensors();
            } else if (cmd == "SET_ALTITUDE") {
                if (args.size() != 2) {
                    response = formatError("Argument manquant.");
                } else {
                    response = setAltitude(args[1]);
                }
            } else if (cmd == "GET_MEASURE") {
                response = getSensorMeasure();
            } else if (cmd == "GET_ERRORS") {
                response = getErrors();
            } else if (cmd == "CLOSE") {
                // Close the client socket
                close(clientSocket);
                cout << "Client disconnected (connection closed)" << endl;
                return;
            } else {
                response = formatError("Commande inconnue.");
            }
        }

        // Send a response back to the client
        write(clientSocket, response.c_str(), response.length());
    }
}

int main() {
    mm = new MeasureModule();

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    // Set up the server address structure
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT); // Port number

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding socket");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Listen for incoming connections (max connexctions: 5)
    if (listen(serverSocket, 5) < 0) {
        perror("Error listening for connections");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    cout << "Daemon listening on port " << PORT << "..." << endl;

    // Accept and handle incoming connections
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Accept a new connection
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Handle the client in a separate function
        thread t(handleClient, clientSocket);
        t.detach();
    }

    // Close the server socket (this part will not be reached)
    close(serverSocket);

    return EXIT_SUCCESS;
}
