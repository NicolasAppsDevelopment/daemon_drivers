#include "packetreader.h"
#include "oxygencalculation.h"
#include <iostream>
#include <cstring>

PacketReader::PacketReader()
{
    reset();
}

BytesArray PacketReader::hexToBytes(const String hex) {
    BytesArray bytes;

    for (UInt i = 0; i < hex.length(); i += 2) {
        String byteString = hex.substr(i, 2);
        Byte byte_ = (Byte) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte_);
    }

    return bytes;
}

FiboxAnswer* PacketReader::processMessage(const BytesArray& buffer)
{
    // read the 2 first bytes of the buffer to determined which part of the message it is
    if (buffer[0] == (Byte)255 && buffer[1] == (Byte)1)
    {
        processReceivedHeader(buffer);
    }
    else if (buffer[0] == (Byte)255 && buffer[1] == (Byte)3)
    {
        BytesArray buf = buffer;
        buf.erase(buf.begin(), buf.begin() + 2); // remove 2 first bytes
        if (processReceivedData(buf)) { // true on new data
            return new FiboxAnswer(temperature, pressure, phase, errors);
        }
    }

    return nullptr;
}

void PacketReader::reset()
{
    this->packageCounter = 0;
    this->deviceResponse = -1;
    this->errors.clear();
    this->temperature = 0;
    this->pressure = 0;
    this->phase = 0;
}

bool PacketReader::processReceivedHeader(const BytesArray& buffer)
{
    int _deviceResponse = ((int)buffer[40] | (int)buffer[41] << 8);
    deviceResponse = _deviceResponse;

    packageCounter = 0;
    return false; // no new o2 data
}

bool PacketReader::processReceivedData(const BytesArray& buffer)
{
    packageCounter++;
    if (deviceResponse == 17)
    {
        return processMeasurement(buffer);
    }
    return false; // no new o2 data
}

bool PacketReader::processReceivedFooter(const BytesArray& buffer)
{
    UShort footer_id = (UShort)((UInt)buffer[2] | (UInt)buffer[3] << 8);
    return false; // no new o2 data
}

std::map<UInt, String> PacketReader::MeasurementErrorDict = {
    {1U, "Le capteur PT100 (température) du Fibox n'est pas connécté"},
    {2U, "Le capteur d'oxygène du Fibox n'est pas/mal détécté (embout en fibre optique et/ou pastille)"},
    {4U, "Le capteur d'oxygène du Fibox (pastille) est mal détécté car l'amplitude du signal de réponse est trop faible. Avez-vous bien mis l'embout en fibre optique et la pastille en face l'un de l'autre ? La pastille est peut-être trop éloigné"},
    {8U, "Défaillance de la carte SD"},
    {16U, "L'amplitude de référence est en dehors des limites"},
    {32U, "La photodiode est saturé. Le capteur est peut-être surexposé à la lumière"},
    {64U, "ADC overflow (Reference)"},
    {128U, "ADC overflow (Signal)"},
    {256U, "ADC overflow (Signal)"},
    {512U, "PME error"},
    {1024U, "Le capteur de pression du Fibox n'a pas été détécté"},
    {2048U, "La température est trop élevé"},
    {4096U, "La carte SD est pleine"},
    {8192U, "Débordement du compteur d'impulsions"},
    {16384U, "Le capteur de température n'est pas disponible"},
    {32768U, "Le capteur de pression n'est pas disponible"},
    {65536U, "La date/heure n'est pas défini"},
    {131072U, "Erreur inconnue"},
    {262144U, "Erreur inconnue"},
    {524288U, "Erreur inconnue"},
    {1048576U, "Erreur inconnue"},
    {2097152U, "Erreur inconnue"},
    {4194304U, "Erreur inconnue"},
    {8388608U, "Erreur inconnue"},
    {16777216U, "Erreur inconnue"},
    {33554432U, "Erreur inconnue"},
    {67108864U, "Erreur inconnue"},
    {134217728U, "Erreur inconnue"},
    {268435456U, "Erreur inconnue"},
    {536870912U, "Erreur inconnue"},
    {1073741820U, "Erreur inconnue"},
    {2147483650U, "Erreur inconnue"}
};

std::map<UInt, String> PacketReader::getMeasurementErrors(UInt error) {
    if (error == 0U)
        return {};
    std::map<UInt, String> measurementErrors;
    double num = static_cast<double>(error);
    for (const auto& pair : MeasurementErrorDict) {
        if (num - static_cast<double>(pair.first) >= 0.0) {
            measurementErrors.insert(std::pair<UInt, String>(pair.first, pair.second));
            num -= static_cast<double>(pair.first);
        }
    }
    return measurementErrors;
}

double PacketReader::toDouble(const BytesArray& buffer) {
    double value = 0.0;
    if (buffer.size() != sizeof(double)) {
        std::cout << "toDouble assert fail" << std::endl;
        return -1;
    }
    memcpy(&value, &buffer[0], std::min(buffer.size(), sizeof(double)));
    return value;
}

UInt PacketReader::toUInt32(const BytesArray& buffer) {
    if (buffer.size() != 4) {
        std::cout << "toUInt32 assert fail" << std::endl;
        return -1;
    }

    UInt res = 0;
    res |= (UInt)buffer[3] << 24;
    res |= (UInt)buffer[2] << 16;
    res |= (UInt)buffer[1] << 8;
    res |= (UInt)buffer[0];

    return res;
}


bool PacketReader::processMeasurement(const BytesArray& buffer)
{
    switch (packageCounter % 7)
    {
        case 2:
            phase = toDouble(buffer);
            break;

        case 4:
            temperature = toDouble(buffer) - 273.15; // From K to °C
            break;

        case 5:
            pressure = toDouble(buffer) * 100.0; // From hPa to Pa
            break;

        case 6:
            errors = getMeasurementErrors(toUInt32(buffer));
            return true; // new data arrived [!]

        default:
            break;
    }

    return false; // no new data
}
