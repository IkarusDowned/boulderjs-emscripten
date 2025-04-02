#include <chrono>
#include <atomic>
#include <iostream>
#include <csignal>
#include <string>
#include <vector>
#include <memory>
#include <argparse/argparse.hpp>
#include "reader/headers/multireadermanager.hpp"

std::unique_ptr<MultiReaderManager> multiReader;

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
    program.add_argument("--file")
        .help("Path to input file. Appended to /var/input/")
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
    std::string filename = program.get("--file");
    std::string filepath = FILEPATH_ROOT + filename;
    std::vector<std::string> filepaths;

    filepaths.push_back(filepath);
    try
    {
        multiReader = std::make_unique<MultiReaderManager>(filepaths);
        multiReader->start();
        while (multiReader->isRunning())
        {
            std::string packet = multiReader->consumePacket();
            //std::cout << "Reading..." << packet << std::endl;
            if (packet == "EOD")
            {
                //std::cout << "Finished!" << std::endl;
                multiReader->stop();
                return 0;
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