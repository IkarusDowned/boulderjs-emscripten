#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include "packet/interfaces/ipacketwriter.hpp"

uint16_t endianTest = 0x1;

class StockTickerWriter
{
private:
    const std::string symbol;
    IPacketWriter &packetWriter;

    const bool isLittleEndian;

public:
    explicit StockTickerWriter(const std::string &symbol, IPacketWriter &packetWriter) : symbol(symbol),
                                                                                         packetWriter(packetWriter),
                                                                                         isLittleEndian(*reinterpret_cast<uint8_t *>(&endianTest) == 0x1)
    {
        if (this->symbol.length() > 2)
        {
            throw std::string("Ticker Symbol must be 2 characeters");
        }
    }

    template <typename T>
    void write(T price)
    {
        const size_t buffSize = sizeof(T);
        char buffer[buffSize];
        std::memcpy(buffer, &price, buffSize);
        if (!isLittleEndian)
        {
            int left = 0;
            int right = buffSize - 1;
            while (left < right)
            {
                char swap = buffer[right];
                buffer[right] = buffer[left];
                buffer[left] = swap;
                ++left;
                --right;
            }
        }
        packetWriter.startPacket();
        packetWriter.write(symbol.c_str(), symbol.length());
        packetWriter.write(buffer, buffSize);
        packetWriter.endPacket();
    }

    void writePrice(float price)
    {
        this->write(price);
    }

    void writeVolume(long volume)
    {
        this->write(volume);
    }

    ~StockTickerWriter()
    {
    }
};