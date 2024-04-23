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

namespace moss {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #define ustring std::string
    #define outs std::cout
    #define errs std::cerr

    /** Returns true is stdin is read from terminal (not redirect) */
    inline bool is_stdin_atty() {
        assert(false && "Check for stdin redirection on non-linux machine is not yet implemented");
        return false; // This way interactive mode will be off
    }

    /**
     * Checks if stderr is redirected to a file
     * @return true if stderr is redirected
     */ 
    inline bool is_stderr_atty() {
        assert(false && "Check for stderr redirection on non-linux machine is not yet implemented");
        return false; // This way interactive mode will be off
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
        static bool is_redir;
        if (!initialized) {
            initialized = true;
            is_redir = ttyname(fileno(stderr)) == nullptr;
        }
        return is_redir;
    }
#endif
}

#endif//_OS_INTERFACE_HPP_