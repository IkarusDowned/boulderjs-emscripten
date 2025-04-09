#include <chrono>
#include <atomic>
#include <iostream>
#include <csignal>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <argparse/argparse.hpp>
#include "headers/multifilereader.hpp"

std::unique_ptr<MultifileReader> multiReader;

static void handleSignal(int signal)
{
    multiReader->stop();
    std::cout << "End called" << std::endl;
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, handleSignal);
    const std::string FILEPATH_ROOT = "/var/input/";
    argparse::ArgumentParser program("multi_file_reader");
    program.add_argument("--files")
        .help("Path to input files. Appended to /var/input/")
        .required();
    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cout << "Error: " << err.what() << "\n\n";
        std::cout << program;
        return 1;
    }
    std::string filenames = program.get("--files");
    std::vector<std::string> filepaths;
    std::istringstream stringStream(filenames);
    std::string filepath;
    while (std::getline(stringStream, filepath, ','))
    {
        filepaths.push_back(FILEPATH_ROOT + filepath);
    }

    int numThreads = filepaths.size();

    try
    {
        multiReader = std::make_unique<MultifileReader>(filepaths);
        auto start = std::chrono::steady_clock::now();
        multiReader->start();
        while (multiReader->isRunning())
        {

            std::string packet = multiReader->consumePacket();
            // std::cout << "Reading..." << packet << std::endl;
            if (packet.find("EOD") != std::string::npos)
            {

                --numThreads;
                std::cout << "Got EOD. " << numThreads << "Remaining..." << std::endl;
                if (numThreads <= 0)
                {
                    // std::cout << "Finished!" << std::endl;
                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    std::cout << duration << std::endl;
                    multiReader->stop();
                    return 0;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}