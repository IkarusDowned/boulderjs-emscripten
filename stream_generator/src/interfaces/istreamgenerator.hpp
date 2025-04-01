#pragma once
/* Interface for the stream generator thread class
 */
class IStreamGenerator
{
public:
    virtual ~IStreamGenerator(){};
    virtual bool initialize() = 0;
    virtual void write(const char *buffer, unsigned int len) = 0;
    virtual void flush() = 0;
};