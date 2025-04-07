FROM gcc:12
RUN apt-get update && apt-get install -y curl && \
    mkdir -p /usr/include/argparse && \
    curl -sSL https://raw.githubusercontent.com/p-ranav/argparse/master/include/argparse/argparse.hpp \
         -o /usr/include/argparse/argparse.hpp


COPY ./lib/. /usr/include/ticker/lib
COPY ./multi_file_reader/. /usr/multi_file_reader/
WORKDIR /usr/multi_file_reader
RUN g++  -std=c++20 -o multi_file_reader \
    -I /usr/include/ticker/lib \
    -I /usr/multi_file_reader/reader/headers \
    main.cpp reader/multireadermanager.cpp -pthread
ENTRYPOINT ["./multi_file_reader"]