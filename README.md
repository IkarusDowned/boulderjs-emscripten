## How to build

### Multi File Reader
`docker build -t multifilereader -f multifilereader.dockerfile .`

### Build the Emscripten JS biding
`docker pull emscripten/emsdk`

on windows:
`MSYS_NO_PATHCONV=1 docker run --rm -v $(pwd):/src -w /src emscripten/emsdk   em++ -v filereader/multifilereader.cpp filereader/bindings.cpp   -o build/multifilereader.js   -I filereader/headers   -I lib   -s USE_PTHREADS=1   -s PTHREAD_POOL_SIZE=4   -s MODULARIZE=1   -s EXPORT_NAME="createMultiReaderModule"   -s ALLOW_MEMORY_GROWTH=1   -std=c++20   -O3   -pthread   --bind`

## How to run
### Create a data file with "packets"
` node ./js/writer.js --file <filename>`

### Multi file Reader
`docker run -v "<input folder>:/var/input" multifilereader --file <filename>`

**Note:** filename gets appended to /var/output/

D:\Projects\boulderjs-emscripten\output