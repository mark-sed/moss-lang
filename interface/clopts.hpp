/**
 * @file clopts.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Command line options handler
 */

#ifndef _CLOPTS_HPP_
#define _CLOPTS_HPP_

#include "args.hpp"
#include "os_interface.hpp"
#include <string>

namespace moss {

/** Command line (terminal) options */
namespace clopts {

inline args::ArgumentParser arg_parser("\b\bMoss Language Interpreter");
inline args::Positional<std::string> file_name(arg_parser, "<file name>", "Input moss program to run");
inline args::ValueFlag<std::string> code(arg_parser, "<code>", "String of moss code to be run", {'e', "execute"});

#ifndef NDEBUG
// Debugging
inline args::ValueFlag<std::string> verbose1(arg_parser, "<csv file::method list>", "Enables prints for logs level 1", {"v", "v1", "verbose", "verbose1"});
inline args::ValueFlag<std::string> verbose2(arg_parser, "<csv file::method list>", "Enables prints for logs level 2", {"vv", "v2", "verbose2"});
inline args::ValueFlag<std::string> verbose3(arg_parser, "<csv file::method list>", "Enables prints for logs level 3", {"vvv", "v3", "verbose3"});
inline args::ValueFlag<std::string> verbose4(arg_parser, "<csv file::method list>", "Enables prints for logs level 4", {"vvvv", "v4", "verbose4"});
inline args::ValueFlag<std::string> verbose5(arg_parser, "<csv file::method list>", "Enables prints for logs level 5", {"vvvvv", "v5", "verbose5"});

inline args::Flag parse_only(arg_parser, "parse-only", "Runs only the parser and does not execute any code", {"parse-only"});
#endif

/**
 * Interpreter argument parsing. It also accepts the program arguments
 * @param argc Number of arguments including the program name
 * @param argv Arguments including the program name
 * @warning This function might terminate the program (after help, version or on error)
 */
void parse_clopts(int argc, const char *argv[]);

/** @return logging (verbose) level set by the user */
int get_logging_level();

/** @return value passed to set verbose level (should be a csv of file::method values) */
std::string get_logging_list();

}

}

#endif//_CLOPTS_HPP_