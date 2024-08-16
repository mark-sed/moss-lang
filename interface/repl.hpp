/**
 * @file repl.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Read-Eval-Print-Loop for moss.
 * This works as a simple text interactive environment for moss.
 */

#ifndef _REPL_HPP_
#define _REPL_HPP_

#include "source.hpp"
#include "parser.hpp"
#include "logging.hpp"

namespace moss {

class Repl {
private:
    SourceFile &src_file;
    Parser parser;

    void output_header();
public:
    Repl(SourceFile &src_file) : src_file(src_file), parser(src_file) {
        LOGMAX("REPL initialized");
    }

    void run();
};

}

#endif//_REPL_HPP_