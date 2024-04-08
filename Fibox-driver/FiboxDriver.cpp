#include "FiboxDriver.h"
#include "../drivererror.h"
#include <iostream>
#include <thread>
#include <unistd.h>

Byte* FiboxDriver::toByteArrayPointer(BytesArray data)
{
    const int len = data.size();
    Byte* res = new Byte[len];

    for (int i = 0; i < len; i++) {
        res[i] = data[i];
    }

    return res;
}

BytesArray FiboxDriver::toByteArray(Byte* data, const int len)
{
    BytesArray res;

    for (int i = 0; i < len; i++) {
        res.push_back(data[i]);
    }

    return res;
}

FiboxDriver::FiboxDriver()
{
    mtx = new std::mutex();
    mtx->lock();

    timeoutWatcherId = 0;
    lastMeasureAnswer = nullptr;

    this->getMeasureTimeout = false;
    this->packetReader = new PacketReader();
    this->devHandle = nullptr;

    this->context = nullptr;
    libusb_init(&context);

    this->criticalError = false;
    this->enableTempFibox = false;
}

void FiboxDriver::initFiboxCommunication()
{
    while (!mtx->try_lock()) {
		mtx->unlock();
	}

    this->packetReader->reset();

    if (this->devHandle != nullptr) {
        libusb_release_interface(this->devHandle, 0);
        libusb_close(this->devHandle);
    }
    this->devHandle = nullptr;
    this->getMeasureTimeout = false;
    this->timeoutWatcherId = 0;

    // Discover connected USB devices
    libusb_device **list;
    libusb_device *found = NULL;
    ssize_t cnt = libusb_get_device_list(context, &list);

    if (cnt < 0) {
        throw DriverError("Impossible d'initialiser la communication avec le Fibox car une erreur est survenue lors de la récupération de la liste d'appareils connéctés.");
    }

    // Try to find Fibox device
    libusb_device_descriptor desc;
    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device *device = list[i];

        if (libusb_get_device_descriptor(device, &desc) != 0) {
            std::cout << "Error while getting device descriptor. Ignoring the device." << std::endl;
        } else {
            if (desc.idProduct == PRODUCT_ID && desc.idVendor == VENDOR_ID) {
                found = device;
                break;
            }
        }
    }

    if (found) {
        int err = libusb_open(found, &devHandle);
        if (err) {
            throw DriverError("Impossible d'initialiser la communication avec le Fibox car une erreur est survenue lors de la tentative de connexion. Assurez vous que ce dernier soit bien alimenté et correctement connécté via son cable USB à la cellule de mesure. Détails : " + String(libusb_strerror(err)) + " (code d'erreur : " + to_string(err) + ").");
        }

        // Get the SerialNumber to initialize the packet writer
        auto* serial = new Byte[33]();
        err = libusb_get_string_descriptor_ascii(devHandle, desc.iSerialNumber, serial, 31);
        if (err >= 0) {
            serial[32] = '\0';
            packetWriter = new PacketWriter((char*)serial);
        }
        else {
            throw DriverError("Impossible d'initialiser la communication avec le Fibox car une erreur est survenu lors de la récupération de son numéro de série.");
        }
    } else {
        throw DriverError("Impossible d'initialiser la communication avec le Fibox car ce dernier est introuvable. Assurez vous que ce dernier soit bien alimenté et correctement connécté via son cable USB à la cellule de mesure.");
    }

    libusb_free_device_list(list, 1);

    // Claim the interface before performing any data transfer
    if (libusb_claim_interface(this->devHandle, 0) < 0) {
        this->criticalError = true;
        throw DriverError("Impossible d'initialiser la communication avec le Fibox car la demande d'accès à son interface a échouée. Serait-il en cours d'utilisation par un autre processus ?");
    }

    // Allocate transfer structure
    libusb_transfer* transfer = libusb_alloc_transfer(0);
    if (!transfer) {
        this->criticalError = true;
        throw DriverError("Impossible d'initialiser la communication avec le Fibox (transfer structure allocation error).");
    }

    // Allocate transfer buffer
    unsigned char buffer[64];
    libusb_fill_bulk_transfer(transfer, devHandle, ENDPOINT_IN, buffer, 64, callback, this, 0);

    // Submit the transfer
    libusb_submit_transfer(transfer);

    this->criticalError = false;
    std::thread handlerThread(&FiboxDriver::handleEvents, this);
    handlerThread.detach();
}

FiboxAnswer* FiboxDriver::getMeasure()
{
    UShort id = packetWriter->getRequestId();
    BytesArray header_packet = packetWriter->prepareGetMeasureRequestHeader(id);
    BytesArray footer_packet = packetWriter->prepareGetMeasureRequestFooter(id);

    try
    {
        sendData(header_packet);
        sendData(footer_packet);
    }
    catch (const DriverError e)
    {
        throw e;
    }

    // Timeout thread
    std::thread timeoutThread(&FiboxDriver::handleTimeout, this, timeoutWatcherId);
    timeoutThread.detach();

    // Wait the new data to arrived
    mtx->lock();
    timeoutWatcherId++;

    // Check wait timeout
    if (getMeasureTimeout) {
        criticalError = true;
        throw DriverError("Impossible de lire les données de mesure du Fibox car le délai de réponse est dépassé.");
    }

    // Check for other sensor error
    if (lastMeasureAnswer->errors.size() != 0) {
        bool throwErrs = false;
        String errors = "Le Fibox a retourné une/plusieurs erreur(s).\\n";
        for (const auto& pair : lastMeasureAnswer->errors) {
            if (pair.first != 1U || enableTempFibox) {
                errors += pair.second + " (code d'erreur : " + to_string(pair.first) + ").\\n";
                throwErrs = true;
            }
        }

        if (throwErrs) {
            errors.pop_back();
            errors.pop_back();
            this->criticalError = true;
            throw DriverError(errors);
        }
    }

    return lastMeasureAnswer;
}

void FiboxDriver::setEnableTempFibox(bool state)
{
    this->enableTempFibox = state;
}

void FiboxDriver::sendData(BytesArray data)
{
    if (!devHandle) {
        throw DriverError("Impossible d'envoyer la demande de mesure au Fibox car la communication s'est mal initialisée (devHandle null).");
    }
    if (this->criticalError) {
        throw DriverError("Impossible d'envoyer la demande de mesure au Fibox car une erreur critique de communication est survenue précédemment (criticalError mode).");
    }

    int transferred;
    int result = libusb_bulk_transfer(devHandle, ENDPOINT_OUT, toByteArrayPointer(data), data.size(), &transferred, 0);
    if (result != 0) {
        this->criticalError = true;
        throw DriverError("Impossible d'envoyer la demande de mesure au Fibox. Assurez vous que ce dernier soit bien alimenté et correctement connécté via son cable USB à la cellule de mesure. Détails : " + String(libusb_strerror(result)) + " (code d'erreur : " + to_string(result) + ").");
    }

    while (!transferred) {
        sleep(1);
    }
}

void FiboxDriver::callback(libusb_transfer* transfer) {
    FiboxDriver* this_ = reinterpret_cast<FiboxDriver*>(transfer->user_data);
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
        return;
    }

    // Handle incoming data
    unsigned char* data = transfer->buffer;
    int actual_length = transfer->actual_length;

    // Process received data
    BytesArray packet = BytesArray();
    for (int i = 0; i < actual_length; ++i) {
        packet.push_back(data[i]);
    }

    FiboxAnswer* answer = this_->packetReader->processMessage(packet);
    if (answer != nullptr) { // complete fibox answer returned
        answer->isTemperatureEnabled = this_->enableTempFibox;
        this_->lastMeasureAnswer = answer;
        this_->mtx->unlock();
    }

    // Re-submit the transfer
    libusb_submit_transfer(transfer);
}

void FiboxDriver::handleTimeout(int id)
{
    // Handle timeout
    this->getMeasureTimeout = false;
    sleep(3);
    if (!mtx->try_lock() && timeoutWatcherId == id) { // if ids are different, the request has been answered before the timeout
        this->getMeasureTimeout = true;
        mtx->unlock();
    }
}

void FiboxDriver::handleEvents()
{
    // Handle events
    while (!criticalError) {
        libusb_handle_events(context);
    }
    printf("Handler thread exited due to a comm error.\n");
}
