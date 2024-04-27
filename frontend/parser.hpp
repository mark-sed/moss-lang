/**
 * @file parser.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Semantical analysis of code and AST generation
 */

#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include "scanner.hpp"
#include "ast.hpp"
#include <memory>

namespace moss {

/**
 * @brief Moss token parsing into AST
 */
class Parser {
private:
    SourceFile &src_file;
    std::unique_ptr<Scanner> scanner;
public:
    Parser(SourceFile &file) : src_file(file), scanner(new Scanner(file)) {}

    AST *parse();
};

}

#endif//_PARSER_HPP_