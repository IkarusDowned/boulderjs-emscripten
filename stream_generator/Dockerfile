FROM gcc:12
COPY ./src/. /usr/src/stream_generator
WORKDIR /usr/src/stream_generator
RUN g++ -std=c++20 -o stream_generator -I . main.cpp -pthread
CMD ["./stream_generator"]