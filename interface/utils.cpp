#include "utils.hpp"
#include "commons.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <memory>

using namespace utils;

std::set<ustring> utils::split_csv_set(ustring csv, char delim) {
    std::set<ustring> splitted;
    ustring value;
    std::stringstream csv_stream(csv);
    while(std::getline(csv_stream, value, delim)){
        utils::trim(value);
        splitted.insert(value);
    }
    return splitted;
}

std::vector<ustring> utils::split_csv(ustring csv, char delim) {
    std::vector<ustring> splitted;
    ustring value;
    std::stringstream csv_stream(csv);
    while(std::getline(csv_stream, value, delim)){
        utils::trim(value);
        splitted.push_back(value);
    }
    return splitted;
}

ustring utils::sanitize(const ustring &text) {
    std::ostringstream oss;
    for (unsigned char c : text) {
        if (c < 0x20) { // ASCII control characters
            switch (c) {
                case '\n': oss << "\\n"; break;
                case '\t': oss << "\\t"; break;
                case '\r': oss << "\\r"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\v': oss << "\\v"; break;
                case '\a': oss << "\\a"; break;
                default:
                    oss << "\\x" << std::hex << std::uppercase
                        << std::setw(2) << std::setfill('0') << (int)c;
            }
        } else if (c == '\"') {
            oss << "\\\"";
        } else if (c == '\\') {
            oss << "\\\\";
        } else {
            oss << c; // printable ASCII or UTF-8 bytes
        }
    }
    return oss.str();
}
