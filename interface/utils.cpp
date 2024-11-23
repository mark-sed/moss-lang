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
    const ustring ESC_CHARS("\t\n\v\a\b\f\r\\");
    ustring sanitized;
    for (ustring::size_type i = 0; i < text.size(); i++) {
        if (ESC_CHARS.find(text[i]) == ustring::npos){
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
