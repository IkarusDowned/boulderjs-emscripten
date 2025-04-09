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
                int bytesRead = packetReader.readPacket(dataBuffer);
                if(bytesRead > 0)
                {
                    std::string packet(dataBuffer.begin(), dataBuffer.end());
                    readerManager.producePacket(packet);
                }
                
            } });
}

void ReaderThread::join()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

void ReaderThread::requestStop()
{
    isStopRequested = true;
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
    packetsBuffer.push(std::move(packet));
}

std::string MultifileReader::consumePacket()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (packetsBuffer.empty()) {
        return "";
    }
    std::string aPacket = std::move(packetsBuffer.front());
    packetsBuffer.pop();
    return aPacket;
}
