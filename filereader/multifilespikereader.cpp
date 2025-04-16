
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <chrono>
#include "thread/headers/threadmanager.hpp"
#include "thread/interfaces/ithread.hpp"
#include "io/headers/filestreamreader.hpp"
#include "packet/headers/asciipacketreader.hpp"
#include "headers/multifilespikereader.hpp"

FileReaderThread::FileReaderThread(const std::vector<std::string> &filePaths) : isRunning(true)
{
    for (auto &filePath : filePaths)
    {
        auto reader = std::make_unique<FileStreamReader>(filePath);
        readers.push_back(std::move(reader));
        auto packetReader = std::make_unique<AsciiPacketReader>(*(readers.back()));
        packetReaders.push_back(std::move(packetReader));
        dataBuffers.push_back(std::vector<char>(MAX_PACKET_SIZE));
        auto pipe = std::make_unique<PacketPipe>();
        packetPipes.push_back(std::move(pipe));
    }
}

FileReaderThread::~FileReaderThread()
{
    
}

int FileReaderThread::initialize()
{
    int readerOk = 1;
    for (auto &reader : readers)
    {
        readerOk &= reader->initialize();
    }
    return readerOk;
}

void FileReaderThread::run()
{
    thread = std::thread([this]()
                         {
        const int initOk = initialize();
        while (initOk && isRunning.load())
        {
            for(int i = 0; i < packetReaders.size(); ++i)
            {
                AsciiPacketReader &packetReader = *packetReaders[i];
                std::vector<char>& dataBuffer = dataBuffers[i];
                PacketPipe& packetPipe = *packetPipes[i];
                int bytesRead = packetReader.readPacket(dataBuffer);
                if(bytesRead > 0)
                {
                    std::string packet(dataBuffer.begin(), dataBuffer.end());
                    packetPipe.enqueue(packet);
                }
            }
            
            
        } });
}

void FileReaderThread::requestStop()
{
    isRunning = false;
}

void FileReaderThread::join()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

void PacketPipe::enqueue(const std::string &packet)
{
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(packet);
}

std::string PacketPipe::dequeue()
{
    std::string local = "";
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.size())
        {
            local = queue.front();
            queue.pop();
        }
    }
    return local;
}

ProducerThread::ProducerThread(PacketPipe& pipe, MultifileSpikeReader& readerManager) : pipe(pipe), priceWelford(1.0, 0.5, 4.0), volumeWelford(2, 1, 3), readerManager(readerManager)
{
}


template <typename T>
inline JsonSpike formatSpikeAsJson(const std::string &streamName, const std::string &type, T value, T mean, T stddev, const std::string &timestampMs)
{
    std::ostringstream ss;
    ss << "{";
    ss << "\"type\":\"" << type << "\",";
    ss << "\"value\":" << value << ",";
    ss << "\"mean\":" << mean << ",";
    ss << "\"stddev\":" << stddev << ",";
    ss << "\"timestamp\":" << timestampMs;
    ss << "}";
    JsonSpike spike;
    spike.json = ss.str();
    spike.streamName = streamName;
    return spike;
}

void ProducerThread::updateWelfordAndReportSpike(const std::string &packet)
{
    enum INDEXES
    {
        STREAM_NAME,
        TYPE,
        VALUE,
        TIMESTAMP
    };
    std::vector<std::string> tokens;
    std::stringstream stringStream(packet);
    std::string token;
    while (std::getline(stringStream, token, ':'))
    {
        tokens.push_back(token);
    }

    if (tokens.size() < 4)
    {
        return;
    }

    std::string streamName = tokens[STREAM_NAME];
    std::string type = tokens[TYPE];
    std::optional<JsonSpike> jSpike = std::nullopt;

    if (type == "P")
    {
        float price = std::stof(tokens[VALUE]);
        auto spike = priceWelford.updateAndDetectSpike(price);
        if (spike)
        {
            jSpike = formatSpikeAsJson<float>(streamName, "price", spike->value, spike->mean, spike->stddev, tokens[TIMESTAMP]);
        }
    }
    else if (type == "V")
    {
        float volume = std::stof(tokens[VALUE]);
        auto spike = volumeWelford.updateAndDetectSpike(volume);
        if (spike)
        {
            jSpike = formatSpikeAsJson<long>(streamName, "volume", spike->value, spike->mean, spike->stddev, tokens[TIMESTAMP]);
        }
    }

    if (jSpike)
    {
        readerManager.produceSpike(*jSpike);
    }
}

void ProducerThread::run()
{

    thread = std::thread([this]()
                         {
            while (!isStopRequested)
            {
                std::string packet = pipe.dequeue();
                
                if(packet != "")
                {
                    updateWelfordAndReportSpike(packet);
                }
                
            } });
}

void ProducerThread::join()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

void ProducerThread::requestStop()
{
    isStopRequested = true;
}

void MultifileSpikeReader::createThreads(const std::vector<std::string> &filePaths)
{
    
    auto fileReaderThread = std::make_unique<FileReaderThread>(filePaths);
    
    for (int i = 0; i < filePaths.size(); ++i)
    {
        threads.push_back(std::make_unique<ProducerThread>(fileReaderThread->getPipe(i), *this));
    }
    threads.push_back(std::move(fileReaderThread));
}

MultifileSpikeReader::MultifileSpikeReader(const std::vector<std::string> &filePaths, emscripten::val callback) : running(true), jsCallback(callback)
{
    createThreads(filePaths);

    std::vector<IThread *> rawThreads;
    for (const auto &thread : threads)
    {
        rawThreads.push_back(thread.get());
    }
    threadManager = std::make_unique<ThreadManager>(rawThreads);
}

MultifileSpikeReader::~MultifileSpikeReader()
{
    
}

bool MultifileSpikeReader::isRunning()
{
    return running;
}

void MultifileSpikeReader::stop()
{
    running = false;
    threadManager->requestStop();
    
}

void MultifileSpikeReader::wait()
{
    threadManager->wait();
}

void MultifileSpikeReader::start()
{
    threadManager->start();
}

void MultifileSpikeReader::produceSpike(const JsonSpike &spike)
{
    std::unique_lock<std::mutex> lock(mutex);
    spikeQueue.push(spike);
}

void MultifileSpikeReader::drainSpikes()
{
    std::queue<JsonSpike> localQueue;
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::swap(localQueue, spikeQueue);
    }

    while (!localQueue.empty())
    {
        const JsonSpike &spike = localQueue.front();
        if (jsCallback.typeOf().as<std::string>() == "function")
        {
            jsCallback(spike.streamName, spike.json);
        }
        localQueue.pop();
    }
}
