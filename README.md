# About
This project is for the BoulderJS talk on April 24, 2025.
The objective is to show that in specific cases, such as high-frequency data streaming, JS / Node's event loop can cause unpredictable results in the main thread.

This specific use case is modeled on my work with creating high-frequency stock feed and analysis systems from 2007. The general basis is a stream of packets, defined by the stock exchanges, and doing some level of rapid work on them.
This is an EXAMPLE, and not production ready, nor does it attempt to highlight every possible use case.

The file writer simulates a high-frequency lossess packet stream.
The reader takes in multiple file streams and processes spikes 

### Packet Definition
A packet is defined as a string of ASCII data which starts with character code STX and ends with ETX
Ex:
`\x02this is packet data!\x03`

within each of these packets is a "coded" ticker

### Ticker format
There are 3 types of tickers. 
1. EOD - End Of Day
Indicates the stream is done for the day.
Format: `<filename>:EOD`
Ex: `output1.dat:EOD`

2. Volume - Trading volume 
Indicates the trading volume at a given time. Values are whole numbers
Ex: `<filename>:V:<volume>:<creation time ms>`
Ex: `test1.dat:V:745:1744558144450`

3. Price - Current Price
Indicates the price at a given time. Values are decimals
Ex: `<filename>:P:<price>:<creation time ms>`
Ex: `test1.dat:P:3.14:1744558144450`

## Requirements
1. Node version 20 or higher
2. Docker version 28 or better
3. The file writer will create files rapidly (up to a 150mb file in under 3 minutes!) so be prepared to have plenty of disc space if you intend to run the file writer for an extended period of time
4. A system capable of using pthreads

## How to build

### Build the Emscripten JS binding
**One-time step:** create a folder `build/`

`docker pull emscripten/emsdk`
then
`docker run --rm -v $(pwd):/src -w /src emscripten/emsdk em++ -v filereader/multifilespikereader.cpp filereader/bindings.cpp -o build/multifilespikereader.mjs -I filereader/headers -I lib -s "EXPORTED_RUNTIME_METHODS=['FS','NODEFS']" -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=6 -s MODULARIZE=1 -s EXPORT_NAME="createMultiReaderModule" -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -O3 -std=c++20 -pthread --bind --no-entry -lnodefs.js`

Note: on windows add `MSYS_NO_PATHCONV=1` before the docker command. so it looks like: `MSYS_NO_PATHCONV=1 docker run ...`

## How to run
### Create a data file with "packets"
`node /js/node/writer/fileWriter.mjs --file <filePath> --seed <seed number>`
the `--seed` value will generate the same ticker price, volume and delay on output every single time

### Reader
There are two applications that you can use to see the different in output between NodeJS and NodeJS + Emscripten
1. Pure NodeJS implementation
`node js/node/reader/reader.mjs --files <file1>,<file2>,<file3>,...,<fileN>`
3. NodeJS + Emscripten (pthreads)
Once you have built the Emscripten JS layer,
`node js/emscripten/reader/reader.mjs --files <file1>,<file2>,<file3>,...,<fileN>`

**Note:** if you are on windows on bash, you will need to add `MSYS_NO_PATHCONV=1` before running 

## Addendum
- This code is provided as is. Please see the license for more details
- At the time of writing this, Emscripten does not have fully multithreaded or system-call capable functionality for the filesystem. It relies on Node's filesystem which is single-threaded and using it in a multi-threaded manner leads to unexpected behaviour. As a result, the emscripten version uses a single thread to read from multiple files and "pipes" them to the packet reeading threads to simulate multiple file reads going on in parallel. In the future, WASMFS should become the standard filesystem for emscripten and allow for faster native-level file reading.
