#include <emscripten/bind.h>
#include "headers/multifilereader.hpp"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(multiReaderBindings) {
    class_<MultifileReader>("MultifileReader")
        .constructor<std::vector<std::string>>()
        .function("start", &MultifileReader::start)
        .function("stop", &MultifileReader::stop)
        .function("isRunning", &MultifileReader::isRunning)
        .function("consumePacket", &MultifileReader::consumePacket);
}