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
    class IR *currently_visiting;
public:
    virtual IR *visit(class Module &i);
    virtual IR *visit(class Space &i);
    virtual IR *visit(class Class &i);
    virtual IR *visit(class Argument &i);
    virtual IR *visit(class Function &i);
    virtual IR *visit(class Lambda &i);
    virtual IR *visit(class Return &i);
    virtual IR *visit(class Else &i);
    virtual IR *visit(class If &i);
    virtual IR *visit(class Switch &i);
    virtual IR *visit(class Case &i);
    virtual IR *visit(class Catch &i);
    virtual IR *visit(class Finally &i);
    virtual IR *visit(class Try &i);
    virtual IR *visit(class While &i);
    virtual IR *visit(class DoWhile &i);
    virtual IR *visit(class ForLoop &i);
    //virtual IR *visit(class Enum &i);
    virtual IR *visit(class Import &i);
    virtual IR *visit(class Assert &i);
    virtual IR *visit(class Raise &i);
    virtual IR *visit(class Break &i);
    virtual IR *visit(class Continue &i);
    virtual IR *visit(class Annotation &i);
    virtual IR *visit(class BinaryExpr &i);
    virtual IR *visit(class UnaryExpr &i);
    virtual IR *visit(class Multivar &i);
    virtual IR *visit(class TernaryIf &i);
    virtual IR *visit(class Range &i);
    virtual IR *visit(class Call &i);
    virtual IR *visit(class List &i);
    virtual IR *visit(class Dict &i);

    IRVisitor(Parser &parser);
    virtual ~IRVisitor() {};
};

/// Manages passes (Visitors) and visits sub IRs
class PassManager : public IRVisitor {
private:
    std::list<IRVisitor *> passes;
protected:
    template <typename T, typename Container>
    void visit_body(Container& nodes);

    template <typename T, typename Setter>
    T* visit_child(T* old_child, Setter set_func, const char* err_msg = "Child cannot be removed", bool allow_null=false);
public:
    virtual IR *visit(class Module &mod) override;
    virtual IR *visit(class Space &spc) override;
    virtual IR *visit(class Class &cls) override;
    virtual IR *visit(class Argument &a) override;
    virtual IR *visit(class Function &fun) override;
    virtual IR *visit(class Lambda &fun) override;
    virtual IR *visit(class Return &ret) override;
    virtual IR *visit(class Else &els) override;
    virtual IR *visit(class If &i) override;
    virtual IR *visit(class Switch &swt) override;
    virtual IR *visit(class Case &cs) override;
    virtual IR *visit(class Catch &ct) override;
    virtual IR *visit(class Finally &fnl) override;
    virtual IR *visit(class Try &tr) override;
    virtual IR *visit(class While &whl) override;
    virtual IR *visit(class DoWhile &dwhl) override;
    virtual IR *visit(class ForLoop &frl) override;
    virtual IR *visit(class Import &imp) override;
    virtual IR *visit(class Assert &a) override;
    virtual IR *visit(class Raise &r) override;
    virtual IR *visit(class Break &b) override;
    virtual IR *visit(class Continue &c) override;
    virtual IR *visit(class Annotation &a) override;
    virtual IR *visit(class BinaryExpr &be) override;
    virtual IR *visit(class UnaryExpr &ue) override;
    virtual IR *visit(class Multivar &mv) override;
    virtual IR *visit(class TernaryIf &ti) override;
    virtual IR *visit(class Range &r) override;
    virtual IR *visit(class Call &cl) override;
    virtual IR *visit(class List &lst) override;
    virtual IR *visit(class Dict &dct) override;

    /// Adds a new pass to the manager to run
    void add_pass(IRVisitor *p);
    void clear_passes() { passes.clear(); }

    PassManager(Parser &parser) : IRVisitor(parser) {}
};

}
}

#endif//_IR_VISITOR_HPP_