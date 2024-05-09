#include "parser.hpp"
#include "ir.hpp"
#include "ast.hpp"

using namespace moss;
using namespace ir;

Module *Parser::parse(bool is_main) {
    Module *m = new Module(this->src_file.get_module_name(), this->src_file, is_main);
    return m;
}