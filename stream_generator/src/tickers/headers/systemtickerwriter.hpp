#pragma once
#include <string>
#include "packet/interfaces/ipacketwriter.hpp"

class SystemTickerWriter
{
public:
    IPacketWriter &packetWriter;
    explicit SystemTickerWriter(IPacketWriter &packetWriter) : packetWriter(packetWriter)
    {
    }

    void sendEndOfDay() {
        const std::string EOD = "EOD";
        packetWriter.startPacket();
        packetWriter.write(EOD.c_str(), EOD.length());
        packetWriter.endPacket();
    }
};