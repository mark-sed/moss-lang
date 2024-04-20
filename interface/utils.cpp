#include "utils.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <set>
#include <vector>

using namespace utils;

std::set<std::string> utils::split_csv_set(std::string csv, char delim) {
    std::set<std::string> splitted;
    std::string value;
    std::stringstream csv_stream(csv);
    while(std::getline(csv_stream, value, delim)){
        utils::trim(value);
        splitted.insert(value);
    }
    return splitted;
}

std::vector<std::string> utils::split_csv(std::string csv, char delim) {
    std::vector<std::string> splitted;
    std::string value;
    std::stringstream csv_stream(csv);
    while(std::getline(csv_stream, value, delim)){
        utils::trim(value);
        splitted.push_back(value);
    }
    return splitted;
}

std::string utils::sanitize(const std::string &text) {
    const std::string ESC_CHARS("\t\n\v\a\b\f\r\\");
    std::string sanitized;
    for (std::string::size_type i = 0; i < text.size(); i++) {
        if (ESC_CHARS.find(text[i]) == std::string::npos){
            sanitized.push_back(text[i]);
        }
        else{
            sanitized.push_back('\\');
            switch(text[i]){
                case '\n': sanitized.push_back('n');
                break;
                case '\t': sanitized.push_back('t');
                break;
                case '\r': sanitized.push_back('r');
                break;
                case '\v': sanitized.push_back('v');
                break;
                case '\\': sanitized.push_back('\\');
                break;
                default: sanitized += std::to_string(static_cast<int>(text[i]));
                break;
            }
        }
    }
    return sanitized;
}