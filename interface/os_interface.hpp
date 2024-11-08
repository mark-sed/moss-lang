/**
 * @file os_interface.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Functions and macros related to the host operating system
 */

#ifndef _OS_INTERFACE_HPP_
#define _OS_INTERFACE_HPP_

#include <cassert>
#include <iostream>
#include <string>
#include <cstdint>

namespace moss {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #define ustring std::string
    #define outs std::cout
    #define errs std::cerr

    #include <io.h>    // For _isatty() on Windows
    #include <fcntl.h> // For _fileno()

    /** Returns true is stdin is read from terminal (not redirect) */
    inline bool is_stdin_atty() {
        return _isatty(_fileno(stdin));
    }

    /**
     * Checks if stderr is redirected to a file
     * @return true if stderr is redirected
     */ 
    inline bool is_stderr_atty() {
        return _isatty(_fileno(stderr));
    }
#else
    #ifndef __linux__
        #warning "Platform is not Linux nor Windows, treating this as Linux"
    #endif
    
    #include <unistd.h>

    // Linux handles char strings as unicode and nothing extra needs to be
    // done to work with unicode
    #define ustring std::string
    #define outs std::cout
    #define errs std::cerr

    /** Returns true is stdin is read from terminal (not redirect) */
    inline bool is_stdin_atty() {
        return isatty(STDIN_FILENO);
    }

    /**
     * Checks if stderr is redirected to a file
     * @return true if stderr is redirected
     */ 
    inline bool is_stderr_atty() {
        static bool initialized(false);
        static bool is_atty;
        if (!initialized) {
            initialized = true;
            is_atty = ttyname(fileno(stderr)) != nullptr;
        }
        return is_atty;
    }
#endif

namespace opcode {

#define BC_OPCODE_SIZE 1  /// Size of opcode in bytes
using opcode_t = uint8_t;

#define BC_REGISTER_SIZE 4 /// How many bytes does register index take
using Register = uint32_t;

#define BC_STR_LEN_SIZE 4 /// How many bytes does the string size take
using strlen_t = uint32_t;
using StringConst = ustring;

#define BC_ADDR_SIZE 4    /// How many bytes does bytecode address take
using Address = uint32_t;

#define BC_INT_SIZE 8 /// How many bytes does int take
using IntConst = int64_t;

#define BC_FLOAT_SIZE 8 /// How many bytes does float take
using FloatConst = double;

#define BC_BOOL_SIZE 1 /// How many bytes does bool take
using BoolConst = bool;

#define BC_RESERVED_REGS 100 /// Number of global registers reserved for built-ins
#define BC_RESERVED_CREGS 300 /// Number of const registers reserved for built-ins
}

}

#endif//_OS_INTERFACE_HPP_