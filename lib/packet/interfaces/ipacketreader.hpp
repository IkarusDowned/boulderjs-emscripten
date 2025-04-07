#pragma once
#include <vector>

class IPacketReader
{
public:
    virtual ~IPacketReader() {}
    virtual int readPacket(std::vector<char> &outPacket) = 0;
};