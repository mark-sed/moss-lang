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

namespace moss {

#ifdef __linux__
    #include <unistd.h>

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
#else
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
#endif
}

#endif//_OS_INTERFACE_HPP_