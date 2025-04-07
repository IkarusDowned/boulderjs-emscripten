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
        for(auto thread : threads) 
        {
            thread->requestStop();
        }
        
    }

    void start()
    {
        for(auto thread : threads) 
        {
            if (thread->initialize(*this))
            {
                thread->run();
            }
        }
        
    }

    void wait()
    {
        for(auto thread : threads)
        {
            thread->join();
        }
        
    }
};