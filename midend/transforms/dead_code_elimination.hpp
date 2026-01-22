/// 
/// \file dead_code_elimination.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Dead code elimination pass.
/// 

#ifndef _DEAD_CODE_ELIMINATION_HPP_
#define _DEAD_CODE_ELIMINATION_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"

namespace moss {
namespace ir {

/// Does constant folding transformation of IR.
class DeadCodeEliminationPass : public IRVisitor {
public:
    DeadCodeEliminationPass(Parser &parser) : IRVisitor(parser) {}
    
    virtual IR *visit(class Function &fun) override;
    virtual IR *visit(class ForLoop &fl) override;
    virtual IR *visit(class While &whl) override;
    virtual IR *visit(class DoWhile &dwhl) override;
    virtual IR *visit(class If &i) override;
    virtual IR *visit(class Else &e) override;
    virtual IR *visit(class Case &cs) override;
    virtual IR *visit(class Catch &c) override;
    virtual IR *visit(class Finally &f) override;
    virtual IR *visit(class Try &t) override;
};

}
}

#endif//_DEAD_CODE_ELIMINATION_HPP_