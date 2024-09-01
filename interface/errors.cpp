#include "errors.hpp"
#include "scanner.hpp"
#include "logging.hpp"
#include "source.hpp"
#include <iostream>
#include <cstddef>
#include <string>

using namespace moss;
using namespace error;

namespace moss {
namespace error {
namespace colors {
    const char * NO_COLOR="\033[0m";
    const char * BLACK="\033[0;30m";
    const char * GRAY="\033[1;30m";
    const char * RED="\033[0;31m";
    const char * LIGHT_RED="\033[1;31m";
    const char * GREEN="\033[0;32m";
    const char * LIGHT_GREEN="\033[1;32m";
    const char * BROWN="\033[0;33m";
    const char * YELLOW="\033[1;33m";
    const char * BLUE="\033[0;34m";
    const char * LIGHT_BLUE="\033[1;34m";
    const char * PURPLE="\033[0;35m";
    const char * LIGHT_PURPLE="\033[1;35m";
    const char * CYAN="\033[0;36m";
    const char * LIGHT_CYAN="\033[1;36m";
    const char * LIGHT_GRAY="\033[0;37m";
    const char * WHITE="\033[1;37m";
    const char * RESET = "\033[0m";
}
}
}

const char *error::get_code_name(error::ErrorCode code){
    const char *NAMES[] = {
        "None",
        "Runtime",
        "Internal",
        "File access",
        "Unimplemented",
        "Bytecode",
        "Argument",
        "Unknown"
    };
    constexpr int names_size = sizeof(NAMES)/sizeof(char *);
    if(static_cast<int>(code) < names_size){
        return NAMES[static_cast<int>(code)];
    }
    return "Unknown";
}

void error::error(error::ErrorCode code, const char *msg, File *src_f, bool exit){
    errs << error::colors::colorize(error::colors::WHITE) << "moss: " << error::colors::reset()
            << error::colors::colorize(error::colors::LIGHT_RED) << (exit ? "fatal error" : "error") << error::colors::reset() 
            << " (" << error::get_code_name(code) << "): ";
    if (src_f) {
        errs << src_f->get_name() << ": ";
    }
    errs << msg << "." << std::endl;
    if(exit){
        error::exit(code);
    }
}

ustring error::format_error(diags::Diagnostic msg) {
    std::stringstream ss;
    const char *bar = "      | ";
    
    SourceInfo info = msg.token->get_src_info();

    ss << error::colors::colorize(error::colors::WHITE) << "moss: " << error::colors::reset()
       << error::colors::colorize(error::colors::LIGHT_RED) << "error" << error::colors::reset() << ": "
       << error::colors::colorize(error::colors::WHITE) << msg.src_f.get_name() << ":" 
       << info.get_lines().first+1 << ":" << info.get_cols().first+1 << error::colors::reset() << ":\n";

    ss << bar << msg.msg << "." << error::colors::colorize(error::colors::GRAY) << " [EDx" << std::hex << std::uppercase << msg.id << std::nouppercase << std::dec << "]" << error::colors::reset() << std::endl;

    const long LINE_LEN_PRE = 100; // Max length of line to be displayed, but will be cut at first
    const long LINE_LEN_POST = 20;

    std::string curr_line = msg.scanner->get_src_text()[info.get_lines().first];
    unsigned col_start = info.get_cols().first;
    unsigned col_end = info.get_cols().second;
    // Append space to take place for a new line which was removed and is at
    // fault here. This makes sure that the token error indicator (^) is
    // pointing right after the token
    if (col_end >= curr_line.size())
        curr_line.push_back(' ');
    if(curr_line.size() > LINE_LEN_PRE+LINE_LEN_POST) {
        size_t start = 0;
        if(info.get_cols().first > LINE_LEN_PRE) {
            start = col_start - LINE_LEN_PRE;
        }
        size_t end = col_end + LINE_LEN_POST;
        if(end > curr_line.size()-1)
            end = curr_line.size()-1;
        curr_line = curr_line.substr(start, end-start);

        col_end -= start;
        if(col_end > end) {
            col_end = end;
        }
        col_start -= start;
    }

    curr_line.insert(col_end, error::colors::colorize(error::colors::RESET));
    curr_line.insert(col_start, error::colors::colorize(error::colors::RED));

    ss << bar << std::endl << std::setfill(' ') << std::setw(sizeof(bar)-3) << info.get_lines().first+1 << " | " << curr_line << std::endl;
    ustring underline = ustring(col_end - col_start, '^');
    ss << bar << std::setfill(' ') << std::setw(col_start + sizeof(bar) - 1) << error::colors::colorize(error::colors::RED) << underline << error::colors::reset() << std::endl;

    return ss.str();
}

void error::error(diags::Diagnostic msg) {
    errs << format_error(msg);
    error::exit(error::ErrorCode::RUNTIME);
}

void error::warning(const char *msg) {
    /* TODO: Check if no-warn is set */
    errs << colors::colorize(colors::PURPLE) << "warning: " << colors::reset() << msg << "." << std::endl;
}

[[noreturn]] void error::exit(error::ErrorCode code) {
    LOG1("Exiting program with code " << code);
    std::exit(code);
}