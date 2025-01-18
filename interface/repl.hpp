/// 
/// \file repl.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Read-Eval-Print-Loop for moss.
/// This works as a simple text interactive environment for moss.
/// 

#ifndef _REPL_HPP_
#define _REPL_HPP_

#include "source.hpp"
#include "parser.hpp"
#include "logging.hpp"

namespace moss {

/// \brief Read-Eval-Print-Loop interface for moss
/// REPL is textual, but user-friendly interface for scripting is moss.
/// It reads user input line/declaration by line/declaration, then parses it,
/// interprets it and prints the result.
/// It has also some special commands for working with repl and if it encounters
/// an exception then it prints it and continues evaluation all new lines.
class Repl {
private:
    SourceFile &src_file;
    Parser parser;

    void output_header();
public:
    Repl(SourceFile &src_file) : src_file(src_file), parser(src_file) {
        LOGMAX("REPL initialized");
    }
    ~Repl() {}
 
    /// \brief Runs REPL
    /// This is the main loop with reading, evaluating and printing
    /// \return Exit code of interpreter
    int run();
};

}

#endif//_REPL_HPP_