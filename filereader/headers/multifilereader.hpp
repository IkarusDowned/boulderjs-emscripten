#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "thread/headers/threadmanager.hpp"
#include "thread/interfaces/ithread.hpp"
#include "io/headers/filestreamreader.hpp"
#include "packet/headers/asciipacketreader.hpp"

const int MAX_PACKET_SIZE = 1000;

class MultifileReader;

class ReaderThread : public IThread
{

    std::thread thread;
    std::atomic<bool> isStopRequested = false;
    FileStreamReader reader;
    AsciiPacketReader packetReader;
    MultifileReader &readerManager;

    std::vector<char> dataBuffer;

public:
    explicit ReaderThread(const std::string &filePath, MultifileReader &readerManager);
    virtual ~ReaderThread();
    virtual int initialize(const IThreadManager &threadManager) override;
    virtual void run() override;

    virtual void join() override;
    virtual void requestStop() override;
};

class MultifileReader
{

private:
    std::unique_ptr<ThreadManager> threadManager;
    std::vector<std::unique_ptr<IThread>> threads;
    std::mutex mutex;
    std::condition_variable condition;
    std::vector<std::string> packetsBuffer;
    int readOffset;
    std::atomic<bool> running;

    void createThreads(const std::vector<std::string> &filePaths);

public:
    explicit MultifileReader(const std::vector<std::string> &filePaths);
    ~MultifileReader();

    void stop();
    void start();
    bool isRunning();
    void producePacket(std::string packet);
    std::string consumePacket();
};
