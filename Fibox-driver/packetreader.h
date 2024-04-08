#ifndef PACKETREADER_H
#define PACKETREADER_H

#include "oxygencalculation.h"
#include <map>
#include "../types.h"
#include "FiboxAnswer.h"
#include <mutex>

/**
 * PacketReader - Fibox driver packet reader class
 * It implements the methods to read and process packets received from the Fibox device
 */
class PacketReader
{
private:
    // read packet (func return true if new data)
    
    /**
     * @brief The package counter
     */
    int packageCounter;

    /**
     * @brief The response code of the Fibox device
     */
    int deviceResponse;

    /**
     * @brief Process and read the received header
     *
     * @param buffer The received buffer
     * @return true if new data is available
     */
    bool processReceivedHeader(const BytesArray& buffer);

    /**
     * @brief Process and read the received data
     *
     * @param buffer The received buffer
     * @return true if new data is available
     */
    bool processReceivedData(const BytesArray& buffer);

    /**
     * @brief Process and read the received footer
     *
     * @param buffer The received buffer
     * @return true if new data is available
     */
    bool processReceivedFooter(const BytesArray& buffer);

    /**
     * @brief Process and read the measurement data
     *
     * @param buffer The received buffer
     * @return true if new data is available
     */
    bool processMeasurement(const BytesArray& buffer);
    
    // data
    double phase;
    double temperature;
    double pressure;
    std::map<UInt, String> errors;

    /**
     * @brief The measurement error dictionary
     */
    static std::map<UInt, String> MeasurementErrorDict;

    /**
     * @brief Process the UInt error code to a list of human readable error message
     *
     * @param error The error code
     * @return The list of human readable error message
     */
    std::map<UInt, String> getMeasurementErrors(UInt error);

    // Helpers funcs
    double toDouble(const BytesArray& buffer);
    UInt toUInt32(const BytesArray& buffer);
    BytesArray hexToBytes(const String hex);

public:
    PacketReader();

    /**
     * @brief Process the received buffer
     * It will identify the type of the received packet and process it
     *
     * @param buffer The received buffer
     * @return The Fibox answer object if new measurement data is available (all response packets received and processed), nullptr otherwise
     */
    FiboxAnswer* processMessage(const BytesArray &buffer);

    /**
     * @brief Reset the packet reader
     */
    void reset();
};

#endif // PACKETREADER_H
