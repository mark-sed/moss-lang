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

namespace moss {

#ifdef __linux__
    #include <unistd.h>

    /** Returns true is stdin is read from terminal (not redirect) */
    bool is_stdin_atty() {
        return isatty(STDIN_FILENO);
    }
#else
    /** Returns true is stdin is read from terminal (not redirect) */
    bool is_stdin_atty() {
        assert(false && "Check for stdin redirection on non-linux machine is not yet implemented");
        return false; // This way interactive mode will be off
    }
#endif
}

#endif//_OS_INTERFACE_HPP_