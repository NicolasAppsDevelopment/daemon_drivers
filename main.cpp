#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "STC31-driver/sensirion_common.h"
#include "STC31-driver/sensirion_i2c_hal.h"
#include "STC31-driver/stc3x_i2c.h"
#include "BME680-driver/common.h"
#include <vector>

using namespace std;

#define PORT 12778

string formatError(string error) {
    return "{\"success\": false, \"error\": \"" + error + "\"}\0";
}
string formatSuccess() {
    return "{\"success\": true}\0";
}
string formatSendData(float gas, float temp) {
    return "{\"success\": true, \"data\": {\"gas\": " + to_string(gas) + ", \"temperature\": " + to_string(temp) + "}}\0";
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

// Syntax : INIT
string initSensor() {
    int16_t error = 0;

    /* STC31 init */
    sensirion_i2c_hal_free();
    error = sensirion_i2c_hal_init();
    if (error) {
        return formatError("Impossible d'initialiser la communication avec le capteur STC31. La fonction \"sensirion_i2c_hal_init\" a retourné le code d'erreur : " + to_string(error));
    }

    uint16_t self_test_output;
    error = stc3x_self_test(&self_test_output);
    if (error) {
        return formatError("L'auto-test du capteur STC31 a échoué. La fonction \"stc3x_self_test\" a retourné le code d'erreur : " + to_string(error));
    }

    error = stc3x_set_binary_gas(0x0001);
    if (error) {
        return formatError("La défénition du mode de relève du CO2 a échoué. La fonction \"stc3x_set_binary_gas\" a retourné le code d'erreur : " + to_string(error));
    }

    /* BME680 init */
    i2c_hal_free();
    error = i2c_hal_init();
    if (error) {
        return formatError("Impossible d'initialiser la communication avec le capteur BME680. La fonction \"i2c_hal_init\" a retourné le code d'erreur : " + to_string(error));
    }

    error = bme680_self_test();
    if (error) {
        return formatError("L'auto-test du capteur BME680 a échoué. La fonction \"bme680_self_test\" a retourné le code d'erreur : " + to_string(error));
    }

    return formatSuccess();
}

// Syntax : CALIBRATE <PRESSURE> <HUMIDITY>
string calibrateSensor(string p, string rh) {
    int16_t error = 0;

    int pressure = 0;
    try {
        pressure = stoi(p);
    } catch (...) {
        return formatError("Invalid pressure argument");
    }

    int relativeHumidity = 0;
    try {
        relativeHumidity = stoi(rh);
    } catch (...) {
        return formatError("Invalid humidity argument");
    }

    uint16_t hum = relativeHumidity * 65535 / 100;
    error = stc3x_set_relative_humidity(hum);
    if (error) {
        return formatError("Error executing stc3x_set_relative_humidity(): " + to_string(error));
    }

    uint16_t pres = pressure;
    error = stc3x_set_pressure(pres);
    if (error) {
        return formatError("Error executing stc3x_set_pressure(): " + to_string(error));
    }

    return formatSuccess();
}

// Syntax : MEASURE
string getSensorMeasure() {
    int16_t error = 0;

    uint16_t gas_ticks;
    uint16_t temperature_ticks;

    float gas;
    float temperature;

    error = stc3x_measure_gas_concentration(&gas_ticks, &temperature_ticks);
    if (error) {
        return formatError("Error executing stc3x_measure_gas_concentration(): " + to_string(error));
    }

    gas = 100 * ((float)gas_ticks - 16384.0) / 32768.0;
    temperature = (float)temperature_ticks / 200.0;


    float temperature2;
    float humidity;
    float pressure;
    float gas2;

    error = bme680_get_measure(&temperature2, &humidity, &pressure, &gas2);
    if (error) {
        return formatError("Error executing bme680_get_measure(): " + to_string(error));
    }

    cout << "--------------------" << endl;
    cout << " GAS: " << gas << endl;
    cout << " TEMP: " << temperature << endl;
    cout << " TEMP2: " << temperature2 << endl;
    cout << " HUMID: " << humidity << endl;
    cout << " PRESS: " << pressure << endl;
    cout << " GAS2: " << gas2 << endl;

    return formatSendData(gas, temperature);
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

            if (cmd == "INIT") {
                response = initSensor();
            } else if (cmd == "CALIBRATE") {
                if (args.size() != 3) {
                    response = formatError("Missing arguments");
                } else {
                    response = calibrateSensor(args[1], args[2]);
                }
            } else if (cmd == "MEASURE") {
                response = getSensorMeasure();
            } else if (cmd == "CLOSE") {
                // Close the client socket
                close(clientSocket);
                cout << "Client disconnected (connection closed)" << endl;
                return;
            } else {
                response = formatError("Unknown command");
            }
        }

        // Send a response back to the client
        write(clientSocket, response.c_str(), response.length());
    }
}

int main() {
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
        handleClient(clientSocket);
    }

    // Close the server socket (this part will not be reached in this simple example)
    close(serverSocket);

    return EXIT_SUCCESS;
}
