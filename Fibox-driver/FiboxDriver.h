#pragma once

#include "../types.h"
#include "packetreader.h"
#include "packetwriter.h"
#include "libusb-1.0/libusb.h"
#include "../MeasureConfig.h"
#include <mutex>

#define VENDOR_ID   0x00FF
#define PRODUCT_ID  0x00FF
#define ENDPOINT_IN 0x81                // Endpoint for data IN
#define ENDPOINT_OUT 0x01               // Endpoint for data OUT

/**
 * @brief FiboxDriver - Fibox driver class
 * It implements the Fibox communication protocol to communicate with the Fibox device
 */
class FiboxDriver
{
private:
    /**
     * @brief The libusb device handle of the Fibox device
     */
    libusb_device_handle* devHandle;

    /**
     * @brief The libusb context of the Fibox device
     */
    libusb_context* context;

    /**
     * @brief The packet reader object
     */
    PacketReader* packetReader;

    /**
     * @brief The packet writer object
     */
    PacketWriter* packetWriter;

    /**
     * @brief Convert a BytesArray to a Byte pointer
     *
     * @param data The BytesArray to convert
     * @return The Byte pointer
     */
    Byte* toByteArrayPointer(BytesArray data);

    /**
     * @brief Convert a Byte pointer to a BytesArray
     *
     * @param data The Byte pointer to convert
     * @param len The length of the Byte pointer
     * @return The BytesArray
     */
    BytesArray toByteArray(Byte* data, const int len);

    /**
     * @brief Callback function that handle responses (by a libusb transfer) from the Fibox device
     *
     * @param transfer The libusb transfer object
     */
    static void callback(libusb_transfer* transfer);

    /**
     * @brief Handle the timeout if a request to the Fibox device is not answered after 3 seconds
     * Called in a thread
     *
     * @param id The id of the transfer
     */
    void handleTimeout(int id);

    /**
     * @brief Loop to handle the events of the Fibox device
     * Called in a thread
     */
    void handleEvents();

    /**
     * @brief The critical error flag
     * True in case of a critical error during the communication.
     */
    bool criticalError;

    /**
     * @brief The flag to get the measure timeout
     * True if the measure request is not answered in time.
     */
    bool getMeasureTimeout;

    /**
     * @brief The flag to enable the temperature measure from the Fibox device
     * True to enable, false otherwise.
     * False will also ignore errors related to the temperature sensor (even if the temp. sensor isn't plug in).
     */
    bool enableTempFibox;

    /**
     * @brief Send data to the Fibox device
     * Must be call in a try instruction
     * 
     * @param data The data to send
     */
    void sendData(BytesArray data);

    /**
     * @brief Mutex to lock the access to the Fibox device
     * It prevent multiple access to the Fibox device
     */
    std::mutex* mtx;

    /**
     * @brief The timeout watcher id
     * It's used to check if a timeout occured
     */
    int timeoutWatcherId;

    /**
     * @brief The last measure answer pointer
     */
    FiboxAnswer* lastMeasureAnswer;

public:
    /**
     * @brief Construct a new Fibox Driver object
     * It initializes the reader/writer objects and the libusb context
     */
    FiboxDriver();

    /**
     * @brief Initialize the Fibox communication
     * Must be call in a try instruction
     */
    void initFiboxCommunication();

    /**
     * @brief Get the measure from the Fibox device
     * Must be call in a try instruction
     * 
     * @return The Fibox answer pointer
     */
    FiboxAnswer* getMeasure();

    /**
     * @brief Set the enable temperature sensor flag
     * 
     * @param state The state of the flag
     */
    void setEnableTempFibox(bool state);

};
