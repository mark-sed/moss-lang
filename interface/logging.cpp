#include "logging.hpp"
#include "commons.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>

using namespace moss;

BaseLogger::BaseLogger() : disable(false), log_everything(false), logging_level{0} {

}

BaseLogger::~BaseLogger() {
    for(auto s: streams){
        s->flush();
    }
}

Logger::Logger() : BaseLogger() {
    streams.push_back(&errs);
}

Logger::~Logger() {
    
}

Logger &Logger::get() {
    static Logger instance;
    return instance;
}

void Logger::debug(unsigned level, const ustring &file_func, const ustring &message) {
    if (disable || level > logging_level) {
        return;
    }

    auto match_f = [file_func](ustring f) {
        return std::regex_match(f, std::regex(file_func));
    };
    // Check if function has logging enabled
    if (!log_everything) {
        if(std::find_if(begin(enabled), end(enabled), match_f) == enabled.end()){
            return;
        }
    }
    // Output log to all registered streams
    for (auto s: streams) {
        (*s) << std::boolalpha << file_func << ": " << message << std::endl;
    }
}