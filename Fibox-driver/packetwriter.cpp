#include "packetwriter.h"
#include "../types.h"

PacketWriter::PacketWriter(String devId) {
    this->requestId = 0;
    this->deviceId = toBytes((UInt)stoi(devId.substr(4)));
}

BytesArray PacketWriter::toBytes(UInt value, bool twoBytesMode) {
    UInt a = value & 0xFF;
    UInt b = (value >> 8) & 0xFF;

    if (twoBytesMode) {
        BytesArray res{(Byte)a ,(Byte)b};
        return res;
    }

    UInt c = (value >> 16) & 0xFF;
    UInt d = (value >> 24) & 0xFF;

    BytesArray res{(Byte)d ,(Byte)a ,(Byte)b ,(Byte)c};
    return res;
}

UShort PacketWriter::getRequestId() const
{
    return this->requestId;
}

UShort PacketWriter::generateRequestId()
{
    UShort max = 65535;
    {
        if (requestId > max - 2) {
            this->requestId = 0;
        } else {
            this->requestId++;
        }
        return requestId;
    }
}

BytesArray PacketWriter::prepareGetMeasureRequestHeader(UShort id)
{
    BytesArray packet;

    // Send message type code (request header = FF 01 01)
    packet.push_back((Byte)255);
    packet.push_back((Byte)1);
    packet.push_back((Byte)1);

    // Send a part of the device ID
    for (size_t i = 0; i < deviceId.size(); i++) {
        packet.push_back(deviceId[i]);
    }

    // Send the static 32-bits channel address (00 01 00 00 00 ... 00)
    for (int i = 0; i <= 32; i++) {
        int val = 0;
        if (i == 1) {
            val = 1;
        }

        packet.push_back((Byte)val);
    }

    // Send the request type code for get measure action (10 00)
    packet.push_back((Byte)16);
    packet.push_back((Byte)0);

    // Send the request id
    BytesArray id_packet = toBytes(id, true);
    packet.push_back((Byte)id_packet[0]);
    packet.push_back((Byte)id_packet[1]);

    // Fill the rest will 0
    packet.push_back((Byte)0);
    packet.push_back((Byte)0);
    packet.push_back((Byte)0);
    packet.push_back((Byte)0);

    // generate an other request id for other requests
    this->generateRequestId();

    return packet;
}

BytesArray PacketWriter::prepareGetMeasureRequestFooter(UShort id)
{
    BytesArray packet;

    // Send message type code (request footer = FF 02)
    packet.push_back((Byte)255);
    packet.push_back((Byte)2);

    // Send the request id
    BytesArray id_packet = toBytes(id, true);
    packet.push_back((Byte)id_packet[0]);
    packet.push_back((Byte)id_packet[1]);

    return packet;
}
