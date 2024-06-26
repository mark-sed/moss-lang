#include "logging.hpp"
#include "os_interface.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

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
    // Check if function has logging enabled
    if (!log_everything) {
        if(enabled.find(file_func) == enabled.end()){
            return;
        }
    }
    // Output log to all registered streams
    for (auto s: streams) {
        (*s) << std::boolalpha << file_func << ": " << message << std::endl;
    }
}