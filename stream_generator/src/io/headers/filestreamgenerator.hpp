#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "io/interfaces/istreamgenerator.hpp"

class FileStreamGenerator : public IStreamGenerator
{
    std::string filePath;
    std::ofstream outputFile;

public:
explicit FileStreamGenerator(const std::string &filePath)
        : filePath(filePath)
    {
    }

    virtual bool initialize()
    {
        outputFile.open(filePath);
        return outputFile.is_open();
    }

    virtual ~FileStreamGenerator()
    {
        outputFile.close();
    }

    virtual void write(const char *buffer, unsigned int len)
    {
        outputFile.write(buffer, len);
    }

    virtual void flush()
    {
        outputFile.flush();
    }
};