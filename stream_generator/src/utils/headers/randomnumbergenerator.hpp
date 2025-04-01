#pragma once
#include <random>

class RandomNumberGenerator {
    public:
        explicit RandomNumberGenerator(unsigned int seed)
            : rng(seed) {}
    
        long getLong(long min, long max) {
            std::uniform_int_distribution<long> dist(min, max);
            return dist(rng);
        }
    
        double getFloat(double min, double max) {
            std::uniform_real_distribution<double> dist(min, max);
            return dist(rng);
        }
    
    private:
        std::mt19937 rng;
    };