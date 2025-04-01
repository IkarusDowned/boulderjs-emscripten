#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

class ArgParser {
public:
    explicit ArgParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg.starts_with("--")) {
                std::string key = arg.substr(2);
                if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("--")) {
                    args_[key] = argv[++i];
                } else {
                    flags_[key] = true;      
                }
            } else {
                positional_.push_back(arg); 
            }
        }
    }

    bool hasFlag(const std::string& flag) const {
        return flags_.count(flag) > 0;
    }

    std::string get(const std::string& key, const std::string& defaultVal = "") const {
        auto it = args_.find(key);
        return (it != args_.end()) ? it->second : defaultVal;
    }

    std::vector<std::string> getPositionalArgs() const {
        return positional_;
    }

private:
    std::unordered_map<std::string, std::string> args_;
    std::unordered_map<std::string, bool> flags_;
    std::vector<std::string> positional_;
};