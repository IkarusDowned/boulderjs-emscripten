#pragma once
#include <csignal>
#include <atomic>
#include <iostream>
#include <vector>

#include "thread/interfaces/ithread.hpp"
#include "thread/interfaces/ithreadmanager.hpp"

class ThreadManager : public IThreadManager
{
private:
    std::vector<IThread *> threads;

public:
    explicit ThreadManager(const std::vector<IThread *> &threads) : threads(threads)
    {
    }

    ~ThreadManager()
    {
    }

    void requestStop()
    {
        for (auto itr = this->threads.begin(); itr != this->threads.end(); ++itr)
        {
            (*itr)->requestStop();
        }
    }

    void start()
    {
        for (auto itr = this->threads.begin(); itr != this->threads.end(); ++itr)
        {
            if ((*itr)->initialize(*this))
            {
                (*itr)->run();
            }
        }
    }

    void wait()
    {
        for (auto itr = this->threads.begin(); itr != this->threads.end(); ++itr)
        {
            (*itr)->join();
        }
    }
};