#pragma once
#include <string>
#include <vector>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <atomic>
#include <cstdint>
#include "thread/headers/threadmanager.hpp"
#include "thread/interfaces/ithread.hpp"
#include "io/headers/notifyfilereader.hpp"
#include "packet/headers/asciipacketreader.hpp"

const int MAX_PACKET_SIZE = 1000;

class MultiReaderManager;

class ReaderThread : public IThread
{

    std::jthread thread;
    NotifyFileReader reader;
    AsciiPacketReader packetReader;
    MultiReaderManager &readerManager;

    std::vector<char> dataBuffer;

public:
    explicit ReaderThread(const std::string &filePath, MultiReaderManager &readerManager);
    virtual ~ReaderThread();
    virtual int initialize(const IThreadManager &threadManager);
    virtual void run();

    virtual void join();
    virtual void requestStop();
};

class MultiReaderManager
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
    explicit MultiReaderManager(const std::vector<std::string> &filePaths);
    ~MultiReaderManager();

    void stop();
    void start();
    bool isRunning();
    void producePacket(std::string packet);
    std::string consumePacket();
};
