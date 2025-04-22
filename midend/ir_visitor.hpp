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

    IRVisitor(Parser &parser);
    virtual ~IRVisitor() {};
};

class PassManager : public IRVisitor {
private:
    std::list<IRVisitor *> module_passes;
    std::list<IRVisitor *> space_passes;
    std::list<IRVisitor *> class_passes;
    std::list<IRVisitor *> function_passes;
public:
    virtual void visit(class Module &mod) override;
    virtual void visit(class Space &spc) override;
    virtual void visit(class Class &cls) override;
    virtual void visit(class Function &fun) override;
    virtual void visit(class Lambda &fun) override;

    void add_module_pass(IRVisitor *p);
    void add_space_pass(IRVisitor *p);
    void add_class_pass(IRVisitor *p);
    void add_function_pass(IRVisitor *p);

    PassManager(Parser &parser) : IRVisitor(parser) {}
};

}
}

#endif//_IR_VISITOR_HPP_