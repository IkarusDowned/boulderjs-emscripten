#include "headers/multifilereader.hpp"
#include <string>
#include <vector>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "thread/headers/threadmanager.hpp"
#include "thread/interfaces/ithread.hpp"
#include "io/headers/filestreamreader.hpp"
#include "packet/headers/asciipacketreader.hpp"

ReaderThread::ReaderThread(const std::string &filePath, MultifileReader &readerManager) : dataBuffer(MAX_PACKET_SIZE), readerManager(readerManager), reader(filePath), packetReader(reader)
{
}

ReaderThread::~ReaderThread()
{
}

int ReaderThread::initialize(const IThreadManager &threadManager)
{
    return reader.initialize();
}

void ReaderThread::run()
{

    thread = std::thread([this]()
                         {
            while (!isStopRequested)
            {
                packetReader.readPacket(dataBuffer);
                std::string packet(dataBuffer.begin(), dataBuffer.end());
                readerManager.producePacket(packet);
            } });
}

void ReaderThread::join()
{
    thread.join();
}

void ReaderThread::requestStop()
{
    isStopRequested = true;
    if (thread.joinable())
    {
        thread.join();
    }
}

void MultifileReader::createThreads(const std::vector<std::string> &filePaths)
{
    for (auto filePath : filePaths)
    {

        threads.push_back(std::make_unique<ReaderThread>(filePath, *this));
    }
}

MultifileReader::MultifileReader(const std::vector<std::string> &filePaths) : packetsBuffer(), running(true)
{
    readOffset = 0;
    createThreads(filePaths);
    std::vector<IThread *> rawThreads;
    for (const auto &thread : threads)
    {
        rawThreads.push_back(thread.get());
    }
    threadManager = std::make_unique<ThreadManager>(rawThreads);
}

MultifileReader::~MultifileReader()
{
}

bool MultifileReader::isRunning()
{
    return running;
}

void MultifileReader::stop()
{
    running = false;
    threadManager->requestStop();
    threadManager->wait();
}

void MultifileReader::start()
{
    threadManager->start();
}

void MultifileReader::producePacket(std::string packet)
{
    std::unique_lock<std::mutex> lock(mutex);
    packetsBuffer.push_back(packet);
    condition.notify_all();
}

std::string MultifileReader::consumePacket()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this]() -> bool
                   { return readOffset < packetsBuffer.size(); });

    std::string aPacket = packetsBuffer[readOffset];
    ++readOffset;
    if (readOffset > 1024 && readOffset > packetsBuffer.size() / 2)
    {
        packetsBuffer.erase(packetsBuffer.begin(), packetsBuffer.begin() + readOffset);
        readOffset = 0;
    }

    return aPacket;
}
