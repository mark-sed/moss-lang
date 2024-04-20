/**
 * @file errors.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Error handling
 */

#ifndef _ERRORS_HPP_
#define _ERRORS_HPP_

#include "os_interface.hpp"
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>

namespace moss {
    
/**
 * Namespace holding resources for error and warning handling
 */
namespace error {
    
    /**
     * Namespace for terminal colorization
     */ 
    namespace colors {

        extern const char *NO_COLOR;
        extern const char *BLACK;
        extern const char *GRAY;
        extern const char *RED;
        extern const char *LIGHT_RED;
        extern const char *GREEN;
        extern const char *LIGHT_GREEN;
        extern const char *BROWN;
        extern const char *YELLOW;
        extern const char *BLUE;
        extern const char *LIGHT_BLUE;
        extern const char *PURPLE;
        extern const char *LIGHT_PURPLE;
        extern const char *CYAN;
        extern const char *LIGHT_CYAN;
        extern const char *LIGHT_GRAY;
        extern const char *WHITE;
        extern const char *RESET;

        /**
         * Returns passes in color in case the output is not redirected.
         * If output is redirected then this returns empty string ("")
         * @param color Colors to sanitize
         * @return color if output if not redirected otherwise empty string
         */ 
        inline const char *colorize(const char * color) {
            // Check if stderr is redirected
            if(is_stderr_atty()) {
                return "";
            }
            return color;
        }

        /**
         * Resets set color to default terminal settings
         * @return colors::RESET if output is not redirected otherwise empty string
         */ 
        inline const char *reset() {
            // Check if stderr is redirected
            if(is_stderr_atty()) {
                return "";
            }
            return colors::RESET;
        }
    }

    /**
     * Possible enum codes
     * @note When new code is added its string name should be added also to the get_code_name function
     */
    enum ErrorCode {
        NO_ERROR = 0,  ///< When no error occurred but program had to exit (otherwise return code would be for some error 0)
        INTERNAL,      ///< Internal compiler error (such as unable to allocate memory)
        FILE_ACCESS,   ///< Problem opening/writing/working with users files (not internal config files)
        UNIMPLEMENTED, ///< Problems with instruction
        UNKNOWN,       ///< Unknown error (shouldn't be really used)
    };

    /**
     * Returns name of ErrorCode
     * @param code Error code
     * @return Error code's name
     */
    const char *get_code_name(ErrorCode code);

    /**
     * Function for when fatal error occurres
     * Prints error information passed in and exits with passed in code
     * @param code Code of an error that occurred
     * @param msg Info message to be printed for the user
     * @param exit If true (default), then after the message is printed program exits with code
     */
    void error(error::ErrorCode code, const char *msg, bool exit=true);

    /**
     * Prints warning to std::cerr
     * @param msg Message to print
     */ 
    void warning(const char *msg);

    /**
     * Exits program with passed in code
     * @param code Error code to exit with
     */ 
    [[noreturn]] void exit(error::ErrorCode code);
}

}

#endif//_ERRORS_HPP_