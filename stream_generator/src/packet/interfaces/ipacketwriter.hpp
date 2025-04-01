#pragma once
#include <string>

class IPacketWriter {
public:
    virtual ~IPacketWriter(){}
    virtual void startPacket() = 0;
    virtual void write(const char *buffer, int length) = 0;
    virtual void endPacket() = 0;
};