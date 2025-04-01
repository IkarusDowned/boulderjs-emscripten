#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "interfaces/istreamgenerator.hpp"

class FileStreamGenerator : public IStreamGenerator
{
    std::string filePath;
    std::ofstream outputFile;

public:
    FileStreamGenerator(const std::string &filePath)
        : filePath(filePath)
    {
        std::cout << "Creating File Stream Object" << std::endl;
    }

    virtual bool initialize()
    {
        outputFile.open(filePath);
        return outputFile.is_open();
    }

    virtual ~FileStreamGenerator()
    {
        outputFile.close();
        std::cout << "Destroying File Stream Object" << std::endl;
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