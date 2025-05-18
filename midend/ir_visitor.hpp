/// 
/// \file ir_visitor.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Visitor for IR using visitor pattern
/// 

#ifndef _IR_VISITOR_HPP_
#define _IR_VISITOR_HPP_

#include "ir.hpp"
#include "logging.hpp"
#include "diagnostics.hpp"

namespace moss {

class Parser;

namespace ir {

/// Visitor for visitor design patter for modifying and analyzing IR
class IRVisitor {
protected:
    Parser &parser;
public:
    virtual void visit(class Module &mod) { 
        (void)mod; 
    }
    virtual void visit(class Space &spc) {
        (void)spc;
    }
    virtual void visit(class Class &cls) {
        (void)cls;
    }
    virtual void visit(class Function &fun) {
        (void)fun;
    }
    virtual void visit(class Lambda &fun) {
        (void)fun;
    }
    virtual void visit(class Return &ret) {
        (void)ret;
    }
    virtual void visit(class Else &els) {
        (void)els;
    }
    virtual void visit(class If &i) {
        (void)i;
    }

    IRVisitor(Parser &parser);
    virtual ~IRVisitor() {};
};

/// Manages passes (Visitors) and visits sub IRs
class PassManager : public IRVisitor {
private:
    std::list<IRVisitor *> passes;
public:
    virtual void visit(class Module &mod) override;
    virtual void visit(class Space &spc) override;
    virtual void visit(class Class &cls) override;
    virtual void visit(class Function &fun) override;
    virtual void visit(class Lambda &fun) override;
    virtual void visit(class Return &ret) override;
    virtual void visit(class Else &els) override;
    virtual void visit(class If &i) override;

    /// Adds a new pass to the manager to run
    void add_pass(IRVisitor *p);

    PassManager(Parser &parser) : IRVisitor(parser) {}
};

}
}

#endif//_IR_VISITOR_HPP_