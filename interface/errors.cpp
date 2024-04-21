#include "errors.hpp"
#include "scanner.hpp"
#include "logging.hpp"
#include <iostream>
#include <cstddef>
#include <string>

using namespace moss;

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
        "Internal",
        "File access",
        "Unimplemented",
        "Unknown"
    };
    constexpr int names_size = sizeof(NAMES)/sizeof(char *);
    if(static_cast<int>(code) < names_size){
        return NAMES[static_cast<int>(code)];
    }
    return "Unknown";
}

void error::error(error::ErrorCode code, const char *msg, SourceFile *src_f, bool exit){
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

void error::warning(const char *msg) {
    /* TODO: Check if no-warn is set */
    errs << colors::colorize(colors::PURPLE) << "warning: " << colors::reset() << msg << "." << std::endl;
}

[[noreturn]] void error::exit(error::ErrorCode code) {
    LOG1("Exiting program with code " << code);
    std::exit(code);
}