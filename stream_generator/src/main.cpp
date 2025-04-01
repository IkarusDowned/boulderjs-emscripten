#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <random>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include "io/headers/filestreamgenerator.hpp"
#include "packet/headers/asciipacketwriter.hpp"
#include "tickers/headers/stocktickerwriter.hpp"
#include "tickers/headers/systemtickerwriter.hpp"
#include "utils/headers/randomnumbergenerator.hpp"
#include "utils/headers/argumentparser.hpp"
#include "thread/interfaces/ithread.hpp"
#include "thread/headers/threadmanager.hpp"

static std::atomic<bool> running(true);
static std::mutex mtx;
static std::condition_variable cv;

static void handleSignal(int signal)
{
    running = false;
    cv.notify_all();
}

class SampleThread : public IThread
{
    std::jthread thread;

private:
    RandomNumberGenerator rng;
    StockTickerWriter &stockWriter;
    const IThreadManager *threadManager;

public:
    explicit SampleThread(unsigned int seed, StockTickerWriter &stockWriter) : rng(seed), stockWriter(stockWriter) {}
    virtual ~SampleThread() {}
    virtual void run()
    {
        
        thread = std::jthread([this](std::stop_token st)
                            {
                                unsigned long packetCount = 0;
                while(!st.stop_requested())
                {

                    stockWriter.writePrice(rng.getFloat(1.0f, 100000.0f));
                    stockWriter.writeVolume(rng.getLong(1, 50000));
                    packetCount += 2ul;
                    if(packetCount % 1000ul == 0ul)
                    {
                        std::cout << "Wrote " << packetCount << " packets..." << std::endl;
                    }
                } 
                std::cout << "Wrote " << packetCount << " packets in total..." << std::endl;
            });
        
    }

    virtual void join()
    {
        thread.join();
    }

    virtual void requestStop()
    {
        thread.request_stop();
    }

    virtual int initialize(const IThreadManager &threadManager)
    {
        this->threadManager = &threadManager;
        return true;
    }
};

int main(int argc, char *argv[])
{
    std::signal(SIGINT, handleSignal);
    ArgParser parser(argc, argv);
    std::string filename = parser.get("file", "/var/output/default.txt");
    int seed = std::stoi(parser.get("seed", "42"));
    std::cout << "Seed value: " << seed << std::endl;
    std::cout << "File: " << filename << std::endl;
    FileStreamGenerator f(filename);
    AsciiPacketWriter b(f);
    StockTickerWriter st("NN", b);
    std::vector<IThread *> threads;
    SampleThread t(seed, st);
    threads.push_back(&t);

    try
    {

        if (f.initialize())
        {
            ThreadManager threadManager(threads);
            threadManager.start();
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, []()
                        { return !running.load(); });
            }
            threadManager.requestStop();
            threadManager.wait();
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