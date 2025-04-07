/// 
/// \file errors.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Error handling
/// 

#ifndef _ERRORS_HPP_
#define _ERRORS_HPP_

#include "commons.hpp"
#include "scanner.hpp"
#include "source.hpp"
#include "diagnostics.hpp"
#include "clopts.hpp"
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>

namespace moss {

class SourceFile;

namespace diags {
    class Diagnostic;
}

/// Namespace holding resources for error and warning handling
namespace error {
    
    
    using msgtype = const char *;
    namespace msgs {
        inline msgtype UNKNOWN_SYMBOL = "Unknown symbol \"%s\"";
        inline msgtype FLOAT_NON_DEC_BASE = "Float can be only of base 10";
        inline msgtype INCORRECT_INT_LITERAL = "Invalid %s number literal";
        inline msgtype INCORRECT_FLOAT_LITERAL = "Invalid float literal";
        inline msgtype UNTERMINATED_STRING_LITERAL = "Unterminated string literal";
        inline msgtype UNTERMINATED_FSTRING_EXPR = "Unterminated expression in fstring literal";
        inline msgtype SINGLE_RCURL_IN_FSTRING = "Single '}' is not allowed in fstring literal";
        inline msgtype UNTERMINATED_COMMENT = "Unterminated comment";
        inline msgtype TRAILING_SEPARATOR = "Trailing digit separators";
        inline msgtype MULTIPLE_SEPARATORS = "Multiple connected digit separators";
        inline msgtype EMPTY_FSTRING_EXPR = "Empty fstring expression";
    }

    /// Namespace for terminal colorization 
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

        /// Returns passes in color in case the output is not redirected.
        /// If output is redirected then this returns empty string ("")
        /// \param color Colors to sanitize
        /// \return color if output if not redirected otherwise empty string 
        inline const char *colorize(const char * color) {
            // Check if stderr is redirected
            if(!is_stderr_atty() || !clopts::use_color()) {
                return "";
            }
            return color;
        }

        /// Resets set color to default terminal settings
        /// \return colors::RESET if output is not redirected otherwise empty string 
        inline const char *reset() {
            // Check if stderr is redirected
            if(!is_stderr_atty() || !clopts::use_color()) {
                return "";
            }
            return colors::RESET;
        }
    }

    /// Possible enum codes
    /// \note When new code is added its string name should be added also to the get_code_name function
    enum ErrorCode {
        NO_ERROR = 0,  ///< When no error occurred but program had to exit (otherwise return code would be for some error 0)
        RUNTIME,       ///< Error caused by users program
        INTERNAL,      ///< Internal compiler error (such as unable to allocate memory)
        FILE_ACCESS,   ///< Problem opening/writing/working with users files (not internal config files)
        UNIMPLEMENTED, ///< Problems with instruction
        BYTECODE,      ///< Problems with bytecode
        ARGUMENT,      ///< Problems with argument parsing
        UNKNOWN,       ///< Unknown error (shouldn't be really used)
    };

    /// Returns name of ErrorCode
    /// \param code Error code
    /// \return Error code's name
    const char *get_code_name(ErrorCode code);

    /// Function for when error occurs
    /// Prints error information passed in and might exit with passed in code
    /// \param code Code of an error that occurred
    /// \param msg Info message to be printed for the user
    /// \param src_f Source file
    /// \param exit If true (default), then after the message is printed program exits with code
    void error(error::ErrorCode code, const char *msg, File *src_f=nullptr, bool exit=true);

    /// Prints diagnostics error and exits with RUNTIME error code
    /// \param msg Diagnostics message
    /// \warning This will terminate the program
    [[noreturn]] void error(diags::Diagnostic msg);

    /// Formats Diagnostics error into a string message with colors (unless
    /// output redirection is detected).
    /// \return Formatted Diagnostics error
    ustring format_error(diags::Diagnostic msg);

    /// Prints warning to std::wcerr
    /// \param msg Message to print 
    void warning(const char *msg);

    /// Exits program with passed in code
    /// \param code Error code to exit with 
    [[noreturn]] void exit(error::ErrorCode code);
}

}

#endif//_ERRORS_HPP_