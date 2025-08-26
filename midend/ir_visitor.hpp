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
    virtual void visit(class Module &) {
    }
    virtual void visit(class Space &) {
    }
    virtual void visit(class Class &) {
    }
    virtual void visit(class Function &) {
    }
    virtual void visit(class Lambda &) {
    }
    virtual void visit(class Return &) {
    }
    virtual void visit(class Else &) {
    }
    virtual void visit(class If &) {
    }
    virtual void visit(class Switch &) {
    }
    virtual void visit(class Case &) {
    }
    virtual void visit(class Catch &) {
    }
    virtual void visit(class Finally &) {
    }
    virtual void visit(class Try &) {
    }
    virtual void visit(class While &) {
    }
    virtual void visit(class DoWhile &) {
    }
    virtual void visit(class ForLoop &) {
    }
    //virtual void visit(class Enum &) {
    //}
    virtual void visit(class Import &) {
    }
    virtual void visit(class Assert &) {
    }
    virtual void visit(class Raise &) {
    }
    //virtual void visit(class Break &) {
    //}
    //virtual void visit(class Continue &) {
    //}
    virtual void visit(class Annotation &) {
    }
    virtual void visit(class BinaryExpr &) {
    }
    virtual void visit(class UnaryExpr &) {
    }
    virtual void visit(class Multivar &) {
    }
    virtual void visit(class TernaryIf &) {
    }
    virtual void visit(class Range &) {
    }
    virtual void visit(class Call &) {
    }
    virtual void visit(class List &) {
    }
    virtual void visit(class Dict &) {
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
    virtual void visit(class Switch &swt) override;
    virtual void visit(class Case &cs) override;
    virtual void visit(class Catch &ct) override;
    virtual void visit(class Finally &fnl) override;
    virtual void visit(class Try &tr) override;
    virtual void visit(class While &whl) override;
    virtual void visit(class DoWhile &dwhl) override;
    virtual void visit(class ForLoop &frl) override;
    virtual void visit(class Import &imp) override;
    virtual void visit(class Assert &a) override;
    virtual void visit(class Raise &r) override;
    virtual void visit(class Annotation &a) override;
    virtual void visit(class BinaryExpr &be) override;
    virtual void visit(class UnaryExpr &ue) override;
    virtual void visit(class Multivar &mv) override;
    virtual void visit(class TernaryIf &ti) override;
    virtual void visit(class Range &r) override;
    virtual void visit(class Call &cl) override;
    virtual void visit(class List &lst) override;
    virtual void visit(class Dict &dct) override;

    /// Adds a new pass to the manager to run
    void add_pass(IRVisitor *p);

    PassManager(Parser &parser) : IRVisitor(parser) {}
};

}
}

#endif//_IR_VISITOR_HPP_