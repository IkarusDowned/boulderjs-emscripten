## How to build

### Multi File Reader
`docker build -t multifilereader -f multifilereader.dockerfile .`

### Build the Emscripten JS biding
`docker pull emscripten/emsdk`
then
`docker run --rm -v $(pwd):/src -w /src emscripten/emsdk   em++ -v filereader/multifilereader.cpp filereader/bindings.cpp -o build/multifilereader.js -I filereader/headers -I lib -s EXPORTED_RUNTIME_METHODS=['FS','NODEFS'] -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4 -s MODULARIZE=1 -s EXPORT_NAME="createMultiReaderModule" -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -std=c++20 -O3 -pthread --bind -lnodefs.js`

Note: on windows add `MSYS_NO_PATHCONV=1` before the docker command. so it looks like: `MSYS_NO_PATHCONV=1 docker run ...`

## How to run
### Create a data file with "packets"
`node ./js/util/fileWriter.js --file <filename>`

### Multi file Reader
`docker run -v "$(pwd)<input folder>:/var/input" multifilereader --file <filename>`

**Note:** filename gets appended to /var/input/ . Same rules for running on windows as above

## Some notes:
- Ideally, we should use the linux system/inotify method of reading files. This would allow non-blocking, low level data reading and you can also allow the threaded file polling to be done on the OS level. However, Emscripten does not allow these header files, so we resort to standard byte streaming.
- 
