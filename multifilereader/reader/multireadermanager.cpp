#include "headers/multireadermanager.hpp"
#include <string>
#include <vector>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "thread/headers/threadmanager.hpp"
#include "thread/interfaces/ithread.hpp"
#include "io/headers/notifyfilereader.hpp"
#include "packet/headers/asciipacketreader.hpp"

ReaderThread::ReaderThread(const std::string &filePath, MultiReaderManager &readerManager) : dataBuffer(MAX_PACKET_SIZE), readerManager(readerManager), reader(filePath), packetReader(reader)
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

    thread = std::jthread([this](std::stop_token st)
                          {
            while (!st.stop_requested())
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
    thread.request_stop();
}

void MultiReaderManager::createThreads(const std::vector<std::string> &filePaths)
{
    for (auto filePath : filePaths)
    {

        threads.push_back(std::make_unique<ReaderThread>(filePath, *this));
    }
}

MultiReaderManager::MultiReaderManager(const std::vector<std::string> &filePaths) : packetsBuffer(), running(true)
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

MultiReaderManager::~MultiReaderManager()
{
}

bool MultiReaderManager::isRunning()
{
    return running;
}

void MultiReaderManager::stop()
{
    running = false;
    threadManager->requestStop();
    threadManager->wait();
}

void MultiReaderManager::start()
{
    threadManager->start();
}

void MultiReaderManager::producePacket(std::string packet)
{
    std::unique_lock<std::mutex> lock(mutex);
    packetsBuffer.push_back(packet);
    condition.notify_all();
}

std::string MultiReaderManager::consumePacket()
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
