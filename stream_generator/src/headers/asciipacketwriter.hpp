#pragma once
#include <iostream>
#include "interfaces/ipacketwriter.hpp"
#include "interfaces/istreamgenerator.hpp"

class AsciiPacketWriter : public IPacketWriter
{
private:
    IStreamGenerator &streamGenerator;
    const char PACKET_START = (char)2;
    const char PACKET_END = (char)3;

public:
    AsciiPacketWriter(IStreamGenerator &streamGenerator) : streamGenerator(streamGenerator)
    {
    }

    virtual ~AsciiPacketWriter()
    {
    }

    virtual void startPacket()
    {
        streamGenerator.write(&PACKET_START, 1);
    }

    virtual void write(const char *buffer, int length)
    {
        streamGenerator.write(buffer, length);
    }

    virtual void endPacket()
    {
        streamGenerator.write(&PACKET_END, 1);
        streamGenerator.flush();
    }
};