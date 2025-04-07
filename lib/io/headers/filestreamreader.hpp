#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "io/interfaces/istreamreader.hpp"

class FileStreamReader : public IStreamReader
{
private:
    const std::string filepath;
    const int maxReadAmount = 64 * 1024;
    std::ifstream infileStream;
    std::vector<char> buffer;

public:
    FileStreamReader(const std::string &filepath) : filepath(filepath), buffer(maxReadAmount) {}

    virtual int initialize()
    {
        infileStream.open(filepath, std::ios::binary);
        return infileStream.is_open();
    }

    virtual int readBytes()
    {
        infileStream.read(buffer.data(), buffer.size());
        int count = infileStream.gcount();

        if (count == 0 && infileStream.eof())
        {
            infileStream.clear();
        }

        return count;
    }

    virtual const std::vector<char> &getByteBuffer()
    {
        return this->buffer;
    }
};