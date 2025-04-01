#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include "headers/filestreamgenerator.hpp"
#include "headers/asciipacketwriter.hpp"
#include "headers/stocktickerwriter.hpp"

int main()
{
    FileStreamGenerator f("/var/output/test");
    AsciiPacketWriter b(f);
    try
    {
        StockTickerWriter st("NY", b);
        if (f.initialize())
        {
            st.writePrice(10);
            st.writeVolume(2000);
        }
        else
        {
            std::cout << "Could not open file for writing" << std::endl;
        }
    }
    catch (const std::string &e)
    {
        std::cout << e << std::endl;
    }
    std::cout << "Done!" << std::endl;
    return 0;
}