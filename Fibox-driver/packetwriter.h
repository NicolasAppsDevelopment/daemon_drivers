#ifndef PACKETWRITER_H
#define PACKETWRITER_H

#include "../types.h"

/**
 * PacketWriter - Fibox driver packet writer class
 * It implements the methods to prepare packets data to be sent to the Fibox device
 */
class PacketWriter
{
private:
    /**
    * @brief The Fibox device ID
    */
    BytesArray deviceId;

    /**
    * @brief The current request ID
    * It is a counter that is incremented for each request
    */
    UShort requestId;

    /**
    * @brief Generate a new request ID
    *
    * @return The new request ID
    */
    UShort generateRequestId();

    /**
    * @brief Convert a UInt value to a BytesArray
    *
    * @param value The UInt value to convert
    * @param twoBytesMode If true, the UInt value is converted to a two bytes array
    * @return The BytesArray
    */
    BytesArray toBytes(UInt value, bool twoBytesMode = false);

public:
    /**
    * @brief Construct a new Packet Writer object
    * It also prepare the device ID bytes array from the device ID string given as parameter
    *
    * @param devId The Fibox device ID string
    */
    PacketWriter(String devId);

    UShort getRequestId() const;

    /**
    * @brief Prepare a measurement request header package
    *
    * @param id The request ID
    * @return The BytesArray
    */
    BytesArray prepareGetMeasureRequestHeader(UShort id);

    /**
    * @brief Prepare a measurement request footer package
    *
    * @param id The request ID
    * @return The BytesArray
    */
    BytesArray prepareGetMeasureRequestFooter(UShort id);

};

#endif // PACKETWRITER_H
