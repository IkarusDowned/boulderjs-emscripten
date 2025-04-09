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

int MultifileReader::getPendingPacketCount() const
{
    return pendingPackets.load();
}


void MultifileReader::producePacket(std::string packet)
{
    std::unique_lock<std::mutex> lock(mutex);
    ++pendingPackets;
    packetsBuffer.push(std::move(packet));
    //condition.notify_all();
}

std::string MultifileReader::consumePacket()
{
    std::unique_lock<std::mutex> lock(mutex);
    //condition.wait(lock, [this]() -> bool { 
    //                                        return !packetsBuffer.empty();
    //});
    if(!pendingPackets)
    {
        return "";  //this should not hit as we should be checking the pendingPackets count and we assume there is 1 consumer
    }
    std::string aPacket = std::move(packetsBuffer.front());
    packetsBuffer.pop();
    --pendingPackets;
    //if(pendingPackets % 100000 == 0)
    //    std::cout << pendingPackets << std::endl;
    return aPacket;
}
