/// 
/// \file clopts.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Command line options handler
/// 

#ifndef _CLOPTS_HPP_
#define _CLOPTS_HPP_

#include "args.hpp"
#include "commons.hpp"
#include <string>

namespace moss {

/// Command line (terminal) options
namespace clopts {

inline args::ArgumentParser arg_parser("\b\bMoss Language Interpreter");
inline args::Group interpreter_group(arg_parser, "Interpreter options:");
inline args::Positional<std::string> file_name(interpreter_group, "<file name>", "Input moss program to run");
inline args::ValueFlag<std::string> code(interpreter_group, "<code>", "String of moss code to be run", {'e', "execute"});
// Note: values in here must match those in enum WarningLevels bellow
inline args::ValueFlag<std::string> warning(interpreter_group, "[all, error, ignore]", "Warning level", {'W', "warning"});

// Bytecode flags
inline args::Group bc_group(arg_parser, "Moss bytecode options:");
inline args::ValueFlag<std::string> output(bc_group, "<msb file name>", "Outputs moss bytecode for input program", {'o', "output-msb"});
inline args::Flag dump_text_bc(bc_group, "S", "Outputs bytecode in textual form", {'S', "textual-msb"});
inline args::Flag output_only(bc_group, "output-only", "Outputs bytecode and does not interpret it", {"output-only"});
inline args::Flag annotate_bc(bc_group, "annotate-bc", "Adds comments to bytecode output", {"annotate-bc"});

inline args::Flag input_bc(bc_group, "bytecode", "Treats input file as bytecode file", {"bytecode"});
inline args::Flag print_bc_info(bc_group, "print-bc-header", "Prints bytecode file (.msb) header and exits", {"print-bc-header"});

// Note flags
inline args::Group note_group(arg_parser, "Note and output options:");
inline args::ValueFlag<std::string> note_file(note_group, "<note file name>", "Outputs moss notes to this file", {'O', "output-file"});
inline args::Flag disable_notes(note_group, "disable-notes", "Disables outputting notes (prints will still output)", {'q', "disable-notes"});
inline args::Flag print_notes(note_group, "print-notes", "Outputs notes also to stdout", {'p', "print-notes"});

// GC flags
inline args::Group gc_group(arg_parser, "Garbage collector options:");
// TODO: Maybe make this (and its uses) debug only
inline args::Flag delete_values_on_exit(gc_group, "delete-values-on-exit", "Deletes all values left by GC on exit", {"delete-values-on-exit"});

#ifndef NDEBUG
// Debugging
inline args::Group debugging_group(arg_parser, "Debugging options:");
inline args::ValueFlag<std::string> verbose1(debugging_group, "<csv file::method list>", "Enables prints for logs level 1", {"v", "v1", "verbose", "verbose1"});
inline args::ValueFlag<std::string> verbose2(debugging_group, "<csv file::method list>", "Enables prints for logs level 2", {"vv", "v2", "verbose2"});
inline args::ValueFlag<std::string> verbose3(debugging_group, "<csv file::method list>", "Enables prints for logs level 3", {"vvv", "v3", "verbose3"});
inline args::ValueFlag<std::string> verbose4(debugging_group, "<csv file::method list>", "Enables prints for logs level 4", {"vvvv", "v4", "verbose4"});
inline args::ValueFlag<std::string> verbose5(debugging_group, "<csv file::method list>", "Enables prints for logs level 5", {"vvvvv", "v5", "verbose5"});

inline args::Flag parse_only(debugging_group, "parse-only", "Runs only the parser and does not execute any code", {"parse-only"});

inline args::Flag stress_test_gc(debugging_group, "stress-test-gc", "Runs GC after every allocation", {"stress-test-gc"});

inline args::Flag use_repl_mode(debugging_group, "use-repl-mode", "Runs moss as if it was repl, allowing for redirected input", {"use-repl-mode"});

#endif

inline args::Group interface_group(arg_parser, "Interface options:");

/// Possible values for -W option
enum WarningLevel {
    WL_IGNORE,
    WL_ALL,
    WL_ERROR,
};

/// Interpreter argument parsing. It also accepts the program arguments
/// \param argc Number of arguments including the program name
/// \param argv Arguments including the program name
/// \warning This function might terminate the program (after help, version or on error) 
void parse_clopts(int argc, const char *argv[]);

/// This needs to be called once no resources from clopts will be used.
void deinit();

/// \return Output stream for writing notes
std::ostream &get_note_stream();

/// \return logging (verbose) level set by the user
int get_logging_level();

/// \return note format selected by user or "txt" if none selected
ustring get_note_format();

/// \return value passed to set verbose level (should be a csv of file::method values)
std::string get_logging_list();

/// \return true if colored output should be used
bool use_color();

/// \return Arguments for the interpreted moss program
std::vector<ustring> get_program_args();

/// \return Wargning level selected by user
WarningLevel get_warning_level();

}

}

#endif//_CLOPTS_HPP_