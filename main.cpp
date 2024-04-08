#include <iostream>
#include "types.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "measuremodule.h"
#include "sensormeasure.h"
#include "TcpMessages/TcpRequest.h"
#include "TcpMessages/TcpAnswer.h"

using namespace std;

#define PORT 12778

MeasureModule* mm;

/**
 * @brief Resets the sensors.
 * TCP command syntax : RESET
 */
void resetSensors() {
    mm->reset();
}

/**
 * @brief Sets the configuration of the sensors.
 * TCP command syntax : SET_CONFIG <ALTITUDE> <F1> <M> <DPHI1> <DPHI2> <DKSV1> <DKSV2> <PRESSURE> <CAL0> <CAL2ND> <T0> <T2ND> <O2CAL2ND> <CALIB_IS_HUMID> <HUMID_MODE> <ENABLE_FIBOX_TEMP>
 *
 * @param request The TCP request object.
 * @param answer The TCP answer object.
 */
void setConfig(TcpRequest* request, TcpAnswer* answer) {
    int altitude = 0;
    try {
        altitude = stoi(request->commandArgs[0]);
    }
    catch (...) {
        answer->setError("L'argument de l'altitude est invalide.");
        return;
    }
    
    double f1 = 0;
    double m = 0;
    double dphi1 = 0;
    double dphi2 = 0;
    double dksv1 = 0;
    double dksv2 = 0;

    try {
        f1 = stod(request->commandArgs[1]);
    }
    catch (...) {
        answer->setError("L'argument F1 est invalide.");
        return;
    }

    try {
        m = stod(request->commandArgs[2]);
    }
    catch (...) {
        answer->setError("L'argument M est invalide.");
        return;
    }

    try {
        dphi1 = stod(request->commandArgs[3]);
    }
    catch (...) {
        answer->setError("L'argument DPHI1 est invalide.");
        return;
    }

    try {
        dphi2 = stod(request->commandArgs[4]);
    }
    catch (...) {
        answer->setError("L'argument DPHI2 est invalide.");
        return;
    }

    try {
        dksv1 = stod(request->commandArgs[5]);
    }
    catch (...) {
        answer->setError("L'argument DKSV1 est invalide.");
        return;
    }

    try {
        dksv2 = stod(request->commandArgs[6]);
    }
    catch (...) {
        answer->setError("L'argument DKSV2 est invalide.");
        return;
    }

    double press = 0;
    double cal0 = 0;
    double cal2nd = 0;
    double t0 = 0;
    double t2nd = 0;
    double o2Cal2nd = 0;
    bool humid = false;

    try {
        press = stod(request->commandArgs[7]);
    }
    catch (...) {
        answer->setError("L'argument de la pression est invalide.");
        return;
    }

    try {
        cal0 = stod(request->commandArgs[8]);
    }
    catch (...) {
        answer->setError("L'argument cal0 est invalide.");
        return;
    }

    try {
        cal2nd = stod(request->commandArgs[9]);
    }
    catch (...) {
        answer->setError("L'argument cal2nd est invalide.");
        return;
    }

    try {
        t0 = stod(request->commandArgs[10]);
    }
    catch (...) {
        answer->setError("L'argument t0 est invalide.");
        return;
    }

    try {
        t2nd = stod(request->commandArgs[11]);
    }
    catch (...) {
        answer->setError("L'argument t2nd est invalide.");
        return;
    }

    try {
        o2Cal2nd = stod(request->commandArgs[12]);
    }
    catch (...) {
        answer->setError("L'argument o2Cal2nd est invalide.");
        return;
    }

    try {
        humid = (bool)stoi(request->commandArgs[13]);
    }
    catch (...) {
        answer->setError("L'argument CALIB_IS_HUMID est invalide.");
        return;
    }

    bool enableFiboxTemp = false;
    bool humidMode = false;

    try {
        enableFiboxTemp = (bool)stoi(request->commandArgs[14]);
    }
    catch (...) {
        answer->setError("L'argument ENABLE_FIBOX_TEMP est invalide.");
        return;
    }

    try {
        humidMode = (bool)stoi(request->commandArgs[15]);
    }
    catch (...) {
        answer->setError("L'argument ENABLE_FIBOX_TEMP est invalide.");
        return;
    }

    mm->setConfig(altitude, f1, m, dphi1, dphi2, dksv1, dksv2, press, cal0, cal2nd, t0, t2nd, o2Cal2nd, humid, humidMode, enableFiboxTemp);
}

/**
 * @brief Gets the errors that occurred.
 * TCP command syntax : GET_ERRORS
 *
 * @param answer The TCP answer object.
 */
void getErrors(TcpAnswer* answer) {
    answer->setMeasurementErrorsData(mm->getErrors());
}

/**
 * @brief Gets the sensor measure.
 * TCP command syntax : GET_MEASURE
 *
 * @param answer The TCP answer object.
 */
void getSensorMeasure(TcpAnswer* answer) {
    SensorMeasure* data = mm->get();
    if (data == nullptr) {
        if (mm->isInitialising()) {
            answer->setError("Le dispositif de mesure n'a fini de s'initialiser.", 1);
            return;
        } else {
            answer->setError("Le dispositif de mesure a probablement été intérrompu à la suite d'une erreur. Pour plus d'information, consultez les erreurs avec GET_ERRORS puis tentez de le réinitialiser avec RESET.", 2);
            return;
        }
    }
    else {
        if (!data->isComplete()) {
			answer->setError("Le dispositif de mesure n'a pas fini de s'initialiser.", 1);
        }
        else {
            answer->setMeasurementsData(data);
        }
    }
}

/**
 * @brief Handles a client request.
 *
 * @param clientSocket The client socket.
 */
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
            // Connection closed by the client or lostn
            close(clientSocket);
            return;
        }

        // Null-terminate the received data
        buffer[bytesRead] = '\0';

        // Try to paste the buffer into a TcpRequest object
        try
        {
            TcpRequest* request = new TcpRequest(buffer);
            TcpAnswer* answer = new TcpAnswer(request->id);

            if (request->commandName == "RESET") {
                resetSensors();
            }
            else if (request->commandName == "SET_CONFIG") {
                if (request->commandArgs.size() != 16) {
                    answer->setError("Argument(s) manquant(s).");
                }
                else {
                    setConfig(request, answer);
                }
            }
            else if (request->commandName == "GET_MEASURE") {
                getSensorMeasure(answer);
            }
            else if (request->commandName == "GET_ERRORS") {
                getErrors(answer);
            }
            else if (request->commandName == "CLOSE") {
                // Close the client socket
                close(clientSocket);
                return;
            }
            else {
                answer->setError("Commande inconnue.");
            }

            // Send a response back to the client
            const String response = answer->toString();
            write(clientSocket, response.c_str(), response.length());

            delete request;
            delete answer;
        }
        catch (...)
        {
            perror("Error parsing/dealing with request !!");
        }
    }
}

/**
 * @brief Main function.
 *
 * @return The exit code.
 */
int main() {
    mm = new MeasureModule();

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    // Set up the server address structure
    struct sockaddr_in serverAddress {};
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
        struct sockaddr_in clientAddress {};
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
