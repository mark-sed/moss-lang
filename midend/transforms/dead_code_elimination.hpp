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

/// \brief Does dead code elimination in IR.
/// Removes code after known taken returns in functions and code in blocks
/// after breaks and continues.
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

/// \brief Does dead branch elimination in IR.
/// Removes constructs that are known to not be executed, such as:
/// ```
/// if (false) {}
/// ```
class DeadBranchEliminationPass : public IRVisitor {
public:
    DeadBranchEliminationPass(Parser &parser) : IRVisitor(parser) {}
    
    virtual IR *visit(class While &whl) override;
    // TODO: DoWhile where if false just the block is kept.
    // TODO: ForLoop with empty string, list, dict or range
    virtual IR *visit(class If &i) override;
};


}
}

#endif//_DEAD_CODE_ELIMINATION_HPP_