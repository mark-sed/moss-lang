/// 
/// \file logging.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Debug logging
/// 

#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

#include "commons.hpp"
#include "utils.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <ostream>
#include <set>
#include <algorithm>
#include <ios>
#include <cstring>
#include <map>

namespace moss {

/// Base class for all loggers 
class BaseLogger {
protected:
    bool disable;
    bool log_everything;
    unsigned logging_level;
    std::vector<std::ostream *> streams;
    std::ios_base::fmtflags flags;
public:
    /// Constructor
    BaseLogger(); 
    /// Destructor. 
    /// Flushes streams
    ~BaseLogger();

    /// Setter for disabling
    /// \param disable If true, the logger will be disabled
    void set_disable(bool disable) { this->disable=disable; }

    /// Sets logging level to be printed
    /// \param level Verbosity level
    void set_logging_level(unsigned level) { this->logging_level = level; }

    /// Adds a new initialized ostream to log to 
    void add_stream(std::ostream *stream) { this->streams.push_back(stream); }

    /// Setter for logging everything
    /// \param log_everything new value 
    void set_log_everything(bool log_everything) { this->log_everything = log_everything; }

    /// Setter for flags
    void set_flags(std::ios_base::fmtflags flags) { this->flags = flags; }

    /// Getter for flags 
    std::ios_base::fmtflags get_flags() { return flags; }

    /// Getter for log_everything
    bool is_log_everything() { return this->log_everything; }
};


/// Class for debugging logs 
class Logger : public BaseLogger {
private:
    std::set<ustring> enabled;
public:
    /// Default constructor for get instance
    Logger();
    ~Logger();

    /// \return logger instance  
    static Logger &get();

    /// Debug logging
    /// \param level Verbosity level
    /// \param file_func __FILE__ should be passed here or the file name
    /// \param message Message to print 
    void debug(unsigned level, const ustring &file_func, const ustring &message);

    
    /// Set file::functions to output to log
    /// \param enabled Set of file::function names
    void set_enabled(std::set<ustring> enabled) { this->enabled = enabled; }
};

/// Tabs for logging
#define TAB1 "\t"
#define TAB2 TAB1 TAB1
#define TAB3 TAB2 TAB1
#define TAB4 TAB3 TAB1
#define TAB5 TAB4 TAB1

/// Maximum logging level
#ifndef MAX_LOGGING_LEVEL
#define MAX_LOGGING_LEVEL 5
#endif

/// Helper macro to strip file's path
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/// Logging macro
#ifndef NDEBUG
    /// @param level Verbosity level
    /// @param message Can be even stream
    #define LOG(level, message) if ((level) <= MAX_LOGGING_LEVEL) { \
        std::stringstream out; \
        out.setf(Logger::get().get_flags()); \
        out << message; \
        Logger::get().debug(level, ustring(__FILENAME__)+ustring("::")+ustring(__func__), out.str()); }
    /// Logs whole container
    #define LOG_CONT(level, message, container) if ((level) <= MAX_LOGGING_LEVEL) { \
        std::stringstream out; \
        out.setf(Logger::get().get_flags()); \
        out << message << std::endl; \
        for(auto v: (container)) { out << TAB1 << v << std::endl; } \
        Logger::get().debug(level, ustring(__FILENAME__)+ustring("::")+ustring(__func__), out.str()); }
    /// Logs container of strings which will be sanitized (removes escape sequences)
    #define LOG_CONT_SANITIZE(level, message, container) if ((level) <= MAX_LOGGING_LEVEL) { \
        std::stringstream out; \
        out.setf(Logger::get().get_flags()); \
        out << message << std::endl; \
        for(auto v: (container)) { out << utils::sanitize(v) << std::endl; } \
        Logger::get().debug(level, ustring(__FILENAME__)+ustring("::")+ustring(__func__), out.str()); }
#else
    #define LOG(level, message)
    #define LOG_CONT(level, message, container)
    #define LOG_CONT_SANITIZE(level, message, container)
#endif//NDEBUG

#define LOG1(message) LOG(1, message)
#define LOG2(message) LOG(2, message)
#define LOG3(message) LOG(3, message)
#define LOG4(message) LOG(4, message)
#define LOG5(message) LOG(5, message)
#define LOGMAX(message) LOG(MAX_LOGGING_LEVEL, message)


}

#endif//_LOGGING_HPP_