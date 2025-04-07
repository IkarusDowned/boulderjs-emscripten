#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include "io/interfaces/istreamreader.hpp"

class NotifyFileReader : public IStreamReader
{
private:
    const std::string filepath;
    const int maxReadAmount = 64 * 1024;
    std::ifstream infileStream;
    std::vector<char> buffer;
    int inotifyFd = -1;
    int watchFd = -1;

    void waitForFileChange()
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(inotifyFd, &fds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Block indefinitely until file is modified
        int selectResult = select(inotifyFd + 1, &fds, nullptr, nullptr, &timeout);
        if (selectResult == -1 && errno != EINTR)
        {
            perror("select (inotify wait)");
        }

        // Drain inotify buffer (required to clear the event)
        char buf[4096];
        read(inotifyFd, buf, sizeof(buf));
    }

public:
    NotifyFileReader(const std::string &filepath) : filepath(filepath), buffer(maxReadAmount)
    {
    }

    virtual int initialize()
    {
        infileStream.open(filepath, std::ios::binary);
        bool isOpen = infileStream.is_open();
        if (isOpen)
        {
            inotifyFd = inotify_init1(IN_NONBLOCK);
            if (inotifyFd == -1)
            {
                perror("inotify_init1");
                return 0;
            }
            watchFd = inotify_add_watch(inotifyFd, filepath.c_str(), IN_MODIFY);
            if (watchFd == -1)
            {
                perror("inotify_add_watch");
                close(inotifyFd);
                return 0;
            }
        }
        return isOpen;
    }

    virtual int readBytes()
    {
        infileStream.read(buffer.data(), buffer.size());
        int count = infileStream.gcount();

        if (count > 0)
        {
            return count;
        }

        // No new data, block until something is written
        infileStream.clear();
        waitForFileChange();

        infileStream.seekg(0, std::ios::cur);

        // Try reading again after notification
        infileStream.read(buffer.data(), buffer.size());
        return infileStream.gcount();
    }

    virtual const std::vector<char> &getByteBuffer()
    {
        return this->buffer;
    }
};