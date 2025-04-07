FROM gcc:12
RUN apt-get update && apt-get install -y curl && \
    mkdir -p /usr/include/argparse && \
    curl -sSL https://raw.githubusercontent.com/p-ranav/argparse/master/include/argparse/argparse.hpp \
         -o /usr/include/argparse/argparse.hpp


COPY ./lib/. /usr/include/ticker/lib
COPY ./filereader/. /usr/filereader/
WORKDIR /usr/filereader
# uncomment this to get maximum performance gains out of C++
# RUN g++  -O3 -march=native -fomit-frame-pointer -std=c++20 -o multifilereader \
RUN g++ -std=c++20 -o multifilereader \
    -I /usr/include/ticker/lib \
    -I /usr/filereader/reader/headers \
    main.cpp multifilereader.cpp -pthread
ENTRYPOINT ["./multifilereader"]